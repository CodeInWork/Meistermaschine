#include <Arduino.h>
#include "app.h"

App app;

void setup()
{
    Serial.begin(9600);
    while (!Serial) {
    }

    if (!app.begin()) {
        Serial.println("App init failed");
        while (true) {
        }
    }
}

void loop()
{
    app.update(millis());
}