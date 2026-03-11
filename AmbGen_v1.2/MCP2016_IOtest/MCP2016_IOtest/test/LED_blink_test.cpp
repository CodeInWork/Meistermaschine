#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

#define LED_PIN 0   // GPA0

Adafruit_MCP23X17 mcp;

void setup() {
    Serial.begin(9600);
    Serial.println("Start");

    if (!mcp.begin_I2C()) {
        Serial.println("MCP23017 not found.");
        while (1){
            delay(1000);
            Serial.println("still not found");
        };
    }

    mcp.pinMode(LED_PIN, OUTPUT);
}

void loop() {
    delay(1000);

    mcp.digitalWrite(LED_PIN, HIGH);
    Serial.println("LED ON");

    delay(1000);

    mcp.digitalWrite(LED_PIN, LOW);
    Serial.println("LED OFF");
}