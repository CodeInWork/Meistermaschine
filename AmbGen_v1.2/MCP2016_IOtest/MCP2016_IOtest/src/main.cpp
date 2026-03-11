#include <Arduino.h>
#include "MCP23017Driver.h"
#include "MCP23017Buttons.h"
#include "app.h"

MCP23017Driver mcp(0x20);
MCP23017Buttons buttons(mcp, 5, 20);
App app(mcp, buttons);

void setup()
{
    Serial.begin(9600);
    delay(300);
    Serial.println("Start");

    mcp.begin();

    if (!app.begin()) {
        Serial.println("App init failed");
        while (true) {
        }
    }

    Serial.println("App ready");
}

void loop()
{
    const uint32_t now = millis();
    app.update(now);
}