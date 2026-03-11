#ifndef MCP23017_BUTTONS_H
#define MCP23017_BUTTONS_H

#include <Arduino.h>
#include "MCP23017Driver.h"

namespace Buttons {
    constexpr uint16_t BTN_0  = (1u << MCP::GPA0);
    constexpr uint16_t BTN_1  = (1u << MCP::GPA1);
    constexpr uint16_t BTN_2  = (1u << MCP::GPA2);
    constexpr uint16_t BTN_3  = (1u << MCP::GPA3);
    constexpr uint16_t BTN_4  = (1u << MCP::GPA4);
    constexpr uint16_t BTN_5  = (1u << MCP::GPA5);
    constexpr uint16_t BTN_6  = (1u << MCP::GPA6);
    constexpr uint16_t BTN_7  = (1u << MCP::GPA7);
    constexpr uint16_t BTN_8  = (1u << MCP::GPB0);
    constexpr uint16_t BTN_9  = (1u << MCP::GPB1);
    constexpr uint16_t BTN_10 = (1u << MCP::GPB2);
    constexpr uint16_t BTN_11 = (1u << MCP::GPB3);
    constexpr uint16_t BTN_12 = (1u << MCP::GPB4);
    constexpr uint16_t BTN_13 = (1u << MCP::GPB5);
    constexpr uint16_t BTN_14 = (1u << MCP::GPB6);
    constexpr uint16_t BTN_15 = (1u << MCP::GPB7);
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

    bool begin(uint16_t enabledMask = Buttons::ALL);
    ButtonEvents update();

    uint16_t rawState() const;
    uint16_t stableState() const;
    uint16_t pressedState() const;

private:
    MCP23017Driver& _driver;
    uint16_t _enabledMask;
    uint32_t _pollIntervalMs;
    uint32_t _debounceTimeMs;

    uint16_t _rawState;
    uint16_t _stableState;
    uint32_t _lastPollMs;
    uint32_t _lastChangeMs;
    bool _initialized;
};

#endif