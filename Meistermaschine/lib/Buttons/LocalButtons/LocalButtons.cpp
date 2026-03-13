#include "LocalButtons.h"

LocalButtons::LocalButtons()
    : _stableState(0),
      _rawState(0),
      _lastPollMs(0),
      _lastChangeMs(0)
{
}

bool LocalButtons::begin()
{
    pinMode(LocalButtonPins::BTN_1, INPUT);
    pinMode(LocalButtonPins::BTN_2, INPUT);
    return true;
}

ButtonEvents LocalButtons::update()
{
    ButtonEvents ev{};
    const uint32_t now = millis();

    if ((now - _lastPollMs) < POLL_MS) {
        return ev;
    }
    _lastPollMs = now;

    uint32_t newRaw = 0;
    if (digitalRead(LocalButtonPins::BTN_1)) {
        newRaw |= LocalButtonMask::BTN_1;
    }
    if (digitalRead(LocalButtonPins::BTN_2)) {
        newRaw |= LocalButtonMask::BTN_2;
    }

    if (newRaw != _rawState) {
        _rawState = newRaw;
        _lastChangeMs = now;
        return ev;
    }

    if ((now - _lastChangeMs) < DEBOUNCE_MS) {
        return ev;
    }

    if (_stableState != _rawState) {
        const uint32_t oldState = _stableState;
        _stableState = _rawState;

        ev.pressed = _stableState & ~oldState;
        ev.released = oldState & ~_stableState;
        ev.held = _stableState;
        ev.valid = true;
        return ev;
    }

    ev.held = _stableState;
    ev.valid = true;
    return ev;
}