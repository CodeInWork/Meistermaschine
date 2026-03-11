#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

Adafruit_MCP23X17 mcp;

void setup() {
    Serial.begin(9600);
    delay(500);

    Serial.println("MCP23017 Pin Test");

    if (!mcp.begin_I2C()) {
        Serial.println("MCP23017 not found.");
        while (1);
    }

    Serial.println("MCP23017 ready.");

    // Alle 16 Pins als OUTPUT konfigurieren
    for (int i = 0; i < 16; i++) {
        mcp.pinMode(i, OUTPUT);
        mcp.digitalWrite(i, LOW);
    }
}

void loop() {

    for (int i = 0; i < 16; i++) {

        Serial.print("Testing pin ");
        Serial.println(i);

        mcp.digitalWrite(i, HIGH);
        delay(500);

        mcp.digitalWrite(i, LOW);
    }
}