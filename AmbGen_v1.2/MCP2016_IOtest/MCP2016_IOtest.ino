#include <Arduino.h>
#include "CyMCP23016.h"

#define LED_PIN MCP23016_PIN_GPIO0_0  // Attach LED to Port 0, Pin 0

CyMCP23016 mcp;

void setup() {
    Serial.begin(9600);

    // Init the MCP23016 at the default address.
    mcp.begin();

    // Init the LED pin.
    mcp.pinMode(LED_PIN, OUTPUT);
    mcp.digitalWrite(LED_PIN, LOW);
}

void loop() {
    delay(1000);

    // Turn the LED on, then read back the state to verify.
    mcp.digitalWrite(LED_PIN, HIGH);
    uint8_t state = mcp.digitalRead(LED_PIN);
    Serial.print(F("LED is ));
    Serial.println(state == HIGH ? "ON", "OFF");

    delay(1000);

    // Turn the LED off, then
    mcp.digitalWrite(LED_PIN, LOW);
    state == mcp.digitalRead(LED_PIN);
    Serial.print(F("LED is ));
    Serial.println(state == HIGH ? "ON" : "OFF");
}