#include "control_filter.h"
#include <Arduino.h>
using namespace std;

// ======== Constants ========
#define REF_MIC_PIN  34    // ADC pin for reference mic
#define ERR_MIC_PIN  35    // ADC pin for error mic
#define DAC_SPKR_PIN 25    // DAC output pin for anti-noise speaker

#define SAMPLE_RATE 8000  // Hz

// ======== Global Variables ========
Control_filter ctrl;                                // FxLMS controller
float sec_path[SEC_LEN] = {0.2f, 0.5f, 0.2f, 0.1f}; // Secondary path coefficients, fill with known path values

hw_timer_t * timer = NULL;                          // Hardware timer for fixed sample rate
volatile bool sampleReady = false;                  // Flag set by timer interrupt

// ======== Timer Interrupt ========
// This runs at the precise sample rate (e.g., 8 kHz)
// Just sets a flag for the main loop to process the next sample
void IRAM_ATTR onTimer() {
    sampleReady = true;
}

// ======== Process One Sample ========
// This function shifts the delay lines, updates FxLMS, and returns outputs
void processSample(float x_ref, float d_measured, float &y_out, float &e_out) {
    // --- Shift the reference delay line x(n) ---
    for(int i = FILTER_LEN-1; i>0; i--)
        ctrl.Xn[i] = ctrl.Xn[i-1];
    ctrl.Xn[0] = x_ref;       // newest sample from reference mic

    // --- Set the desired signal at error mic ---
    ctrl.Dn = d_measured;

    // --- Run one FxLMS step ---
    ctrl.step();

    // --- Outputs ---
    y_out = ctrl.Yn;      // Anti-noise output to DAC
    e_out = ctrl.En;      // Error signal for monitoring
}

// ======== Arduino Setup ========
void setup() {
    Serial.begin(115200);  // Serial monitor for debugging
    
    // --- Initialize secondary path (known or measured beforehand) ---
    for(int i = 0; i < SEC_LEN; i++)
        ctrl.Sn[i] = sec_path[i];

    // --- Setup hardware timer for fixed sample rate (SAMPLE_RATE Hz) ---
    timer = timerBegin(0, 80, true);                        // Timer 0, prescaler 80 → 1 MHz tick
    timerAttachInterrupt(timer, &onTimer, true);            // Attach ISR
    timerAlarmWrite(timer, 1000000 / SAMPLE_RATE, true);    // Trigger at SAMPLE_RATE
    timerAlarmEnable(timer);
}

void loop() {
    // Only process a sample when the timer sets the flag
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
        dacWrite(DAC_SPKR_PIN, int(yOut * 255));

        // --- Optional: monitor signals on Serial ---
        Serial.print("Ref: "); Serial.print(refSample, 4);
        Serial.print("\tAntiNoise: "); Serial.print(yOut, 4);
        Serial.print("\tError: "); Serial.println(eOut, 4);
    }
}