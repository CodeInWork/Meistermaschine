#ifndef MCP23017_BUTTONS_H
#define MCP23017_BUTTONS_H

#include <Arduino.h>
#include "MCP23017Driver.h"

namespace Buttons {
    constexpr uint16_t BTN_0  = MCP::bitMask(MCPPin::GPA0);
    constexpr uint16_t BTN_1  = MCP::bitMask(MCPPin::GPA1);
    constexpr uint16_t BTN_2  = MCP::bitMask(MCPPin::GPA2);
    constexpr uint16_t BTN_3  = MCP::bitMask(MCPPin::GPA3);
    constexpr uint16_t BTN_4  = MCP::bitMask(MCPPin::GPA4);
    constexpr uint16_t BTN_5  = MCP::bitMask(MCPPin::GPA5);
    constexpr uint16_t BTN_6  = MCP::bitMask(MCPPin::GPA6);
    constexpr uint16_t BTN_7  = MCP::bitMask(MCPPin::GPA7);
    constexpr uint16_t BTN_8  = MCP::bitMask(MCPPin::GPB0);
    constexpr uint16_t BTN_9  = MCP::bitMask(MCPPin::GPB1);
    constexpr uint16_t BTN_10 = MCP::bitMask(MCPPin::GPB2);
    constexpr uint16_t BTN_11 = MCP::bitMask(MCPPin::GPB3);
    constexpr uint16_t BTN_12 = MCP::bitMask(MCPPin::GPB4);
    constexpr uint16_t BTN_13 = MCP::bitMask(MCPPin::GPB5);
    constexpr uint16_t BTN_14 = MCP::bitMask(MCPPin::GPB6);
    constexpr uint16_t BTN_15 = MCP::bitMask(MCPPin::GPB7);
    constexpr uint16_t ALL    = 0xFFFF;
}

struct ButtonEvents {
    uint16_t pressed = 0;   // gerade neu gedrückt
    uint16_t released = 0;  // gerade neu losgelassen
    uint16_t held = 0;      // aktuell gedrückt
    bool valid = false;     // false bei I2C-Fehler / kein neues Polling
};

class MCP23017Buttons {
public:
    MCP23017Buttons(MCP23017Driver& driver,
                    uint32_t pollIntervalMs = 5,
                    uint32_t debounceTimeMs = 20);

    ButtonEvents update();
    void setButtonMask(uint16_t mask);

private:
    MCP23017Driver& _driver;
    
    uint16_t _buttonMask;

    uint32_t _pollIntervalMs;
    uint32_t _debounceTimeMs;

    uint16_t _rawState;
    uint16_t _stableState;
    
    uint32_t _lastPollMs;
    uint32_t _lastChangeMs;

};

#endif