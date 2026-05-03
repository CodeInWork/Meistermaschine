#ifndef LOCAL_BUTTONS_H
#define LOCAL_BUTTONS_H

#include <Arduino.h>
#include "ButtonUtils.h"
#include "ButtonEvents.h"
#include "HardwareConfig.h"

namespace LocalButtonIds
{
    static constexpr ButtonLayout::ButtonId BTN_1 = 0;
    static constexpr ButtonLayout::ButtonId BTN_2 = 1;
}

class LocalButtons
{
public:
    LocalButtons();

    bool begin();
    ButtonEvents update();

private:
    ButtonLayout::ButtonMask readRawMask() const;

private:
    ButtonLayout::ButtonMask _stableState;
    ButtonLayout::ButtonMask _rawState;

    uint32_t _lastPollMs;
    uint32_t _lastChangeMs;

    static constexpr uint32_t POLL_MS = 5;
    static constexpr uint32_t DEBOUNCE_MS = 20;
};

#endif