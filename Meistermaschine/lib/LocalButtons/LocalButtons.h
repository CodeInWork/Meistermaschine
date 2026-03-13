#ifndef LOCAL_BUTTONS_H
#define LOCAL_BUTTONS_H

#include <Arduino.h>

namespace LocalButtonPins {
    constexpr uint8_t BTN_1 = 5;
    constexpr uint8_t BTN_2 = 6;
}

namespace LocalButtonMask {
    constexpr uint32_t BTN_1 = (1UL << 0);
    constexpr uint32_t BTN_2 = (1UL << 1);
}

struct ButtonEvents {
    uint32_t pressed = 0;
    uint32_t released = 0;
    uint32_t held = 0;
    bool valid = false;
};

class LocalButtons
{
public:
    LocalButtons();

    bool begin();
    ButtonEvents update();

private:
    uint32_t _stableState;
    uint32_t _rawState;
    uint32_t _lastPollMs;
    uint32_t _lastChangeMs;

    static constexpr uint32_t POLL_MS = 5;
    static constexpr uint32_t DEBOUNCE_MS = 20;
};

#endif