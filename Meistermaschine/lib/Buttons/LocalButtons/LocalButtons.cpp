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

ButtonLayout::ButtonMask LocalButtons::readRawMask() const
{
    ButtonLayout::ButtonMask mask = 0;

    if (digitalRead(LocalButtonPins::BTN_1)) {
        mask |= ButtonLayout::ID2Mask(LocalButtonIds::BTN_1);
    }

    if (digitalRead(LocalButtonPins::BTN_2)) {
        mask |= ButtonLayout::ID2Mask(LocalButtonIds::BTN_2);
    }

    return mask;
}

ButtonEvents LocalButtons::update()
{
    ButtonEvents ev{};

    const uint32_t now = millis();

    if ((now - _lastPollMs) < POLL_MS) {
        return ev;
    }

    _lastPollMs = now;

    const ButtonLayout::ButtonMask newRaw = readRawMask();

    if (newRaw != _rawState) {
        _rawState = newRaw;
        _lastChangeMs = now;
        return ev;
    }

    if ((now - _lastChangeMs) < DEBOUNCE_MS) {
        return ev;
    }

    if (_stableState != _rawState) {
        const ButtonLayout::ButtonMask oldState = _stableState;
        _stableState = _rawState;

        ev.pressed  = _stableState & ~oldState;
        ev.released = oldState & ~_stableState;
        ev.held     = _stableState;
        ev.valid    = true;

        return ev;
    }

    ev.held = _stableState;
    ev.valid = true;
    return ev;
}