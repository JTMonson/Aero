#include <Arduino.h>

#define DAC_SPKR_PIN 25
#define SAMPLE_RATE 8000

float sinePhase = 0.0f;

// sweep parameters
float minFreq = 0.0f;
float maxFreq = 1518.0f;
float currentFreq = 1518.0f;

float sweepSpeed = 2.0f;   // Hz change per sample
int direction = -1;        // start sweeping downward

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

    if (sampleReady) {
        sampleReady = false;

        // update frequency
        currentFreq += direction * sweepSpeed;

        // reverse direction at limits
        if (currentFreq <= minFreq) {
            currentFreq = minFreq;
            direction = 1;
        }

        if (currentFreq >= maxFreq) {
            currentFreq = maxFreq;
            direction = -1;
        }

        // sine generator
        float amplitude = 100.0f;
        float offset = 127.0f;

        float dacFloat = offset + amplitude * sinf(sinePhase);

        // advance sine phase
        sinePhase += TWO_PI * currentFreq / SAMPLE_RATE;

        if (sinePhase >= TWO_PI) {
            sinePhase -= TWO_PI;
        }

        if (dacFloat < 0) dacFloat = 0;
        if (dacFloat > 255) dacFloat = 255;

        dacWrite(DAC_SPKR_PIN, (uint8_t)dacFloat);
    }
}