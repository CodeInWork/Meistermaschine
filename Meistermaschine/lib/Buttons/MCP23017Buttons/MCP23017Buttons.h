#ifndef MCP23017_BUTTONS_H
#define MCP23017_BUTTONS_H

#include <Arduino.h>

#include "ButtonEvents.h"
#include "ButtonUtils.h"
#include "MCP23017Driver.h"

class MCP23017Buttons
{
public:
    MCP23017Buttons(
        MCP23017Driver& driver,
        uint8_t firstColumn
    );

    bool begin();
    ButtonEvents update();

private:
    static constexpr uint16_t GPA_BUTTON_MASK = 0x001F;
    static constexpr uint16_t GPB_BUTTON_MASK = 0x1F00;

    static constexpr uint16_t ENABLED_PIN_MASK =
        GPA_BUTTON_MASK | GPB_BUTTON_MASK;

    MCP23017Driver& _driver;
    uint8_t _firstColumn;

    uint16_t _rawState16;
    uint16_t _stableState16;

    uint32_t _lastPollMs;
    uint32_t _lastChangeMs;

    bool readRawState(uint16_t& state);
    ButtonLayout::Coord pinToCoord(uint8_t pin) const;

    static constexpr uint32_t POLL_MS = 5;
    static constexpr uint32_t DEBOUNCE_MS = 20;
};

#endif