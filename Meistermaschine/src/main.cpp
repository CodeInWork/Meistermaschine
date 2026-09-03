#include <Arduino.h>
#include "app.h"
#include "SoftwareConfig.h"

App app;

void setup()
{
    if (SoftwareConfig::DEBUG) {
        Serial.begin(9600);
        //while (!Serial) {}    uncomment for boards with native USB (e.g. Leonardo, Micro, Zero, etc.)
        delay(1000);
        Serial.println(F("Starting MEISTERMASCHINE..."));
    }

    if (!app.begin()) {
        if (SoftwareConfig::DEBUG) {
            Serial.println(F("App init failed"));
        }
        while (true) {
        }
    }

    if (SoftwareConfig::DEBUG) {
        Serial.println(F("Setup complete"));
    }
}

void loop()
{
    app.update(millis());
}