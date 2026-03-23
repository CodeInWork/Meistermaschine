#include <Arduino.h>
#include "app.h"

App app;

void setup()
{
    Serial.begin(9600);
    //while (!Serial) {}    uncomment for boards with native USB (e.g. Leonardo, Micro, Zero, etc.)
    delay(1000);
    Serial.println(F("Starting AmbGen..."));

    if (!app.begin()) {
        Serial.println("App init failed");
        while (true) {
        }
    }

    Serial.println(F("Setup complete"));
}

void loop()
{
    app.update(millis());
}