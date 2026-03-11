#include <Arduino.h>

#define DAC_SPKR_PIN 25
#define SAMPLE_RATE 8000

float sineFreq = 1518.0f;       // Hz
float sinePhase = 0.0f;
const float TWO_PI = 6.28318530718f;

hw_timer_t *timer = NULL;
volatile bool sampleReady = false;

void IRAM_ATTR onTimer() {
    sampleReady = true;
}

void setup() {
    Serial.begin(115200);

    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000000UL / SAMPLE_RATE, true);
    timerAlarmEnable(timer);
}

void loop() {
    // Only run when the timer interrupt signals a new sample time
    if (sampleReady) {
        sampleReady = false;   // reset the flag so we wait for the next timer tick

        // Amplitude of the sine wave (how loud it is)
        // Must stay below ~127 so the DAC doesn't clip
        float amplitude = 100.0f;

        // Offset centers the sine wave in the DAC range
        // ESP32 DAC range is 0–255, so 127 is the midpoint
        float offset = 127.0f;

        // Generate one sample of the sine wave
        // sinf() outputs -1 to +1
        // Multiply by amplitude and add offset to map it into 0–255 range
        float dacFloat = offset + amplitude * sinf(sinePhase);

        // Advance the phase for the next sample
        // This determines the sine wave frequency
        // Formula: phase_step = 2π * frequency / sample_rate
        sinePhase += TWO_PI * sineFreq / SAMPLE_RATE;

        // Wrap phase after one full sine cycle (2π)
        // Prevents the phase variable from growing forever
        if (sinePhase >= TWO_PI) {
            sinePhase -= TWO_PI;
        }

        // Clamp the value to valid DAC range (safety)
        if (dacFloat < 0.0f) dacFloat = 0.0f;
        if (dacFloat > 255.0f) dacFloat = 255.0f;

        // Send the sample to the ESP32 DAC output pin
        // This converts the digital value into an analog voltage
        dacWrite(DAC_SPKR_PIN, (uint8_t)dacFloat);
    }
}