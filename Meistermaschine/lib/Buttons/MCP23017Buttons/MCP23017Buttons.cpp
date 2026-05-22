#include "MCP23017Buttons.h"

MCP23017Buttons::MCP23017Buttons(MCP23017Driver& driver,
                                 ButtonLayout::ButtonId firstButtonId)
    : _driver(driver),
      _firstButtonId(firstButtonId),
      _rawState16(0),
      _stableState16(0),
      _lastPollMs(0),
      _lastChangeMs(0)
{
}

bool MCP23017Buttons::begin()
{
    _driver.begin();

    // all 16 pins input
    if (!_driver.setDirection(0xFFFF)) {
        return false;
    }

    // internal MCP pullups on all 16 pins
    if (!_driver.setPullups(0xFFFF)) {
        return false;
    }

    uint16_t gpio = 0xFFFF;
    if (!_driver.readGPIO(gpio)) {
        return false;
    }

    // active-low buttons:
    // open = 1, pressed = 0
    _rawState16 = static_cast<uint16_t>(~gpio);
    _stableState16 = _rawState16;

    return true;
}

ButtonLayout::ButtonMask MCP23017Buttons::readRawMask()
{
    uint16_t gpio = 0xFFFF;

    if (!_driver.readGPIO(gpio)) {
        return 0;
    }

    const uint16_t pressed16 = static_cast<uint16_t>(~gpio);

    ButtonLayout::ButtonMask mask = 0;

    for (uint8_t i = 0; i < 16; ++i) {
        if (pressed16 & static_cast<uint16_t>(1u << i)) {
            const ButtonLayout::ButtonId buttonId =
                static_cast<ButtonLayout::ButtonId>(_firstButtonId + i);

            if (ButtonLayout::isValid(buttonId)) {
                mask |= ButtonLayout::ID2Mask(buttonId);
            }
        }
    }

    return mask;
}

ButtonEvents MCP23017Buttons::update()
{
    ButtonEvents ev{};

    const uint32_t now = millis();

    if ((now - _lastPollMs) < POLL_MS) {
        return ev;
    }

    _lastPollMs = now;

    const ButtonLayout::ButtonMask newRawMask = readRawMask();

    // only first 16 bits relevant for this single MCP instance
    uint16_t newRaw16 = 0;

    for (uint8_t i = 0; i < 16; ++i) {
        const ButtonLayout::ButtonId buttonId =
            static_cast<ButtonLayout::ButtonId>(_firstButtonId + i);

        if (ButtonLayout::isValid(buttonId) &&
            (newRawMask & ButtonLayout::ID2Mask(buttonId))) {
            newRaw16 |= static_cast<uint16_t>(1u << i);
        }
    }

    if (newRaw16 != _rawState16) {
        _rawState16 = newRaw16;
        _lastChangeMs = now;
        return ev;
    }

    if ((now - _lastChangeMs) < DEBOUNCE_MS) {
        return ev;
    }

    if (_stableState16 != _rawState16) {
        const uint16_t oldState = _stableState16;
        _stableState16 = _rawState16;

        const uint16_t pressed16  = _stableState16 & ~oldState;
        const uint16_t released16 = oldState & ~_stableState16;

        for (uint8_t i = 0; i < 16; ++i) {
            const ButtonLayout::ButtonId buttonId =
                static_cast<ButtonLayout::ButtonId>(_firstButtonId + i);

            if (!ButtonLayout::isValid(buttonId)) {
                continue;
            }

            const ButtonLayout::ButtonMask bit = ButtonLayout::ID2Mask(buttonId);

            if (pressed16 & static_cast<uint16_t>(1u << i)) {
                ev.pressed |= bit;
            }

            if (released16 & static_cast<uint16_t>(1u << i)) {
                ev.released |= bit;
            }

            if (_stableState16 & static_cast<uint16_t>(1u << i)) {
                ev.held |= bit;
            }
        }

        ev.valid = true;
        return ev;
    }

    for (uint8_t i = 0; i < 16; ++i) {
        const ButtonLayout::ButtonId buttonId =
            static_cast<ButtonLayout::ButtonId>(_firstButtonId + i);

        if (ButtonLayout::isValid(buttonId) &&
            (_stableState16 & static_cast<uint16_t>(1u << i))) {
            ev.held |= ButtonLayout::ID2Mask(buttonId);
        }
    }

    ev.valid = true;
    return ev;
}