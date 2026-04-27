/*
===============================================================================
  Active Noise Control (FxLMS) - ESP32 (Arduino)

  What this file does:
    - Runs an FxLMS controller at a fixed sampling rate using an ESP32 hardware
      timer interrupt as the sample “clock”.
    - ISR (interrupt) only sets a flag.
    - loop() does the heavy work once per tick: read ADCs, run FxLMS step,
      write DAC, optionally print.

  Key timing idea:
    - ESP32 timer base clock: 80 MHz
    - timerBegin(timer_number, prescaler, countUp)
        * timer_number: which hardware timer to use (0..3 typically)
        * prescaler: divides the 80 MHz timer clock
        * countUp: true = count up, false = count down
    - With prescaler = 80:
        80 MHz / 80 = 1 MHz  →  1 tick = 1 microsecond (1 µs)

  Setting the sample rate:
    - timerAlarmWrite(timer, alarm_value_ticks, autoReload)
        * alarm_value_ticks: how many timer ticks between interrupts
        * autoReload: true = periodic, false = one-shot
    - alarm_value_ticks = 1,000,000 / SAMPLE_RATE
      Example: SAMPLE_RATE = 8000 Hz → 1,000,000 / 8000 = 125 µs per sample

  Interrupt behavior:
    - The timer hardware interrupts the CPU every alarm period.
    - timerAttachInterrupt(timer, &onTimer, edge)
      attaches your ISR (onTimer) to the timer event.
    - Keep ISR short: no Serial, no heavy DSP.

  pio run --target upload

===============================================================================
*/

#include "control_filter.h"
#include <Arduino.h>
// SD card includes
#include "FS.h"
#include "SD_MMC.h"
using namespace std;

// ======== Constants ========
#define REF_MIC_PIN  34     // ADC pin for reference mic
#define ERR_MIC_PIN  35     // ADC pin for error mic
#define DAC_SPKR_PIN 25     // DAC output pin for anti-noise speaker

#define SAMPLE_RATE 20000   // Hz

// ======== Global Variables ========
Control_filter ctrl;                                // FxLMS controller instance
float sec_path[SEC_LEN] = {1.0f, 1.0f, 1.0f, 1.0f}; // Secondary path coefficients, fill with known path values

hw_timer_t *timer = NULL;           // ESP32 hardware timer handle pointer
volatile bool sampleReady = false;  // Set by ISR, used in loop()

// ======== Timer ISR Interrupt ========
// Runs at SAMPLE_RATE, only signals main loop
void IRAM_ATTR onTimer() {
    sampleReady = true;     // Signal to process one sample
}

// ======== Process One Sample ========
// This function shifts the delay lines, updates FxLMS, and returns outputs
void processSample(float x_ref, float d_measured, float &y_out, float &e_out) {
    // Shift the reference delay line x(n)
    for(int i = FILTER_LEN-1; i>0; i--) {
        ctrl.Xn[i] = ctrl.Xn[i-1];
    }
    ctrl.Xn[0] = x_ref;       // newest sample from reference mic

    // Set the desired / measured signal at error mic
    ctrl.Dn = d_measured;

    // Run one FxLMS update step 
    ctrl.step();

    // Outputs
    y_out = ctrl.Yn;      // Anti-noise output to DAC
    e_out = ctrl.En;      // Error signal for monitoring
}

// ======== Arduino Setup ========
void setup() {
    Serial.begin(115200);  // Serial monitor for debugging
    
    // Load known secondary path model into controller
    for(int i = 0; i < SEC_LEN; i++)
        ctrl.Sn[i] = sec_path[i];

  // --- SD Card Initialization ---
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD failed");
    // Optionally, halt or retry
  } else if (SD_MMC.cardType() == CARD_NONE) {
    Serial.println("No SD card");
  } else {
    Serial.println("SD success");
    // Write CSV header if file does not exist
    if (!SD_MMC.exists("/data.csv")) {
      File file = SD_MMC.open("/data.csv", FILE_WRITE);
      if (file) {
        file.println("millis,ref_mic,anti_noise,error_mic");
        file.close();
      }
    }
  }
    // --- Setup hardware timer for fixed sample rate ---
    
    // Configure timer: 80 MHz / 80 prescaler = 1 MHz -> 1 microsecond per tick
    timer = timerBegin(0, 80, true);

    // Call onTimer() whenever the timer alarm fires
    timerAttachInterrupt(timer, &onTimer, true);

    // Fire every (1e6 / SAMPLE_RATE) ticks (microseconds)
    timerAlarmWrite(timer, 1000000UL / SAMPLE_RATE, true);

    // Start the sampling timer
    timerAlarmEnable(timer);
}

void loop() {
    // Check if the timer interrupt has signaled
    if(sampleReady) {
        sampleReady = false;

        // --- Read analog signals ---
        float refSample = analogRead(REF_MIC_PIN) / 4095.0f;  // normalize 12-bit ADC to 0..1
        float errSample = analogRead(ERR_MIC_PIN) / 4095.0f;

        // --- Process FxLMS ---
        float yOut, eOut;
        processSample(refSample, errSample, yOut, eOut);

        // --- Output anti-noise to speaker via DAC ---
        // Scale yOut (assumed 0..1) to DAC 0..255
        // Clamp output to DAC range (0..1)
        if (yOut < 0.0f) yOut = 0.0f;
        if (yOut > 1.0f) yOut = 1.0f;
        
        // Scale to 8-bit DAC (0..255)
        dacWrite(DAC_SPKR_PIN, (uint8_t)(yOut * 255));

        // Debug printing, disable once verified working
        static int counter = 0;
        if (++counter >= 100) {   // print every 100 samples
          counter = 0;
          Serial.print("Ref: "); Serial.print(refSample, 4);
          Serial.print("\tAntiNoise: "); Serial.print(yOut, 4);
          Serial.print("\tError: "); Serial.println(eOut, 4);
        }

        // --- SD Card Logging ---
        File file = SD_MMC.open("/data.csv", FILE_APPEND);
        if (file) {
          file.print(millis());
          file.print(",");
          file.print(refSample, 6);
          file.print(",");
          file.print(yOut, 6);
          file.print(",");
          file.println(eOut, 6);
          file.close();
        } else {
          // Optionally print error
          // Serial.println("SD write failed");
        }
    }
}