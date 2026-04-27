#include <Arduino.h>
#include "FS.h"
#include "SD_MMC.h"

using namespace std;

// ==== Constats ====
#define CLK 14  // S0
#define CMD 15  // 
#define D0 2
#define D1 4
#define D2 12
#define D3 13

File file;

// ======== Arduino Setup ========
void setup() {
    Serial.begin(115200);

    // SD_MMC.setFrequency(400000);  // 400 kHz

    // Start SD card
    if (!SD_MMC.begin("/sdcard", true)) {
        Serial.println("SD failed");
        return;
    }

    // Check if card exists
    if (SD_MMC.cardType() == CARD_NONE) {
        Serial.println("No SD card");
        return;
    }

    Serial.println("SD success");
}

// // ======== Arduino Loop ========
void loop() {
    File file = SD_MMC.open("/data.txt", FILE_APPEND);

    if (file) {
        file.println(millis());
        file.close();

        Serial.println("Write success");
    } else {
        Serial.println("Write failed");
    }

    delay(50);
}