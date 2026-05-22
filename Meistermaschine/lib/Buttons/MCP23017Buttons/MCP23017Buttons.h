#ifndef MCP23017_BUTTONS_H
#define MCP23017_BUTTONS_H

#include <Arduino.h>
#include "MCP23017Driver.h"
#include "ButtonUtils.h"
#include "ButtonEvents.h"

class MCP23017Buttons
{
public:
    explicit MCP23017Buttons(MCP23017Driver& driver,
                             ButtonLayout::ButtonId firstButtonId = 0);

    bool begin();
    ButtonEvents update();

private:
    ButtonLayout::ButtonMask readRawMask();

private:
    MCP23017Driver& _driver;
    ButtonLayout::ButtonId _firstButtonId;

    uint16_t _rawState16;
    uint16_t _stableState16;

    uint32_t _lastPollMs;
    uint32_t _lastChangeMs;

    static constexpr uint32_t POLL_MS = 5;
    static constexpr uint32_t DEBOUNCE_MS = 20;
};

#endif