#include "MCP23017Buttons.h"

MCP23017Buttons::MCP23017Buttons(
    MCP23017Driver& driver,
    uint8_t firstColumn
)
    : _driver(driver),
      _firstColumn(firstColumn),
      _rawState16(0),
      _stableState16(0),
      _lastPollMs(0),
      _lastChangeMs(0)
{
}

bool MCP23017Buttons::begin()
{
    _driver.begin();

    // All MCP pins remain inputs.
    if (!_driver.setDirection(0xFFFF)) {
        return false;
    }

    // Pull-ups are needed only on the ten connected button pins:
    // GPA0-GPA4 and GPB0-GPB4.
    if (!_driver.setPullups(ENABLED_PIN_MASK)) {
        return false;
    }

    uint16_t initialState = 0;

    if (!readRawState(initialState)) {
        return false;
    }

    _rawState16 = initialState;
    _stableState16 = initialState;

    _lastPollMs = millis();
    _lastChangeMs = _lastPollMs;

    return true;
}

bool MCP23017Buttons::readRawState(uint16_t& state)
{
    uint16_t gpio = 0xFFFF;

    if (!_driver.readGPIO(gpio)) {
        return false;
    }

    // Active-low buttons:
    // open    = GPIO bit 1
    // pressed = GPIO bit 0
    //
    // Invert so that internally:
    // released = 0
    // pressed  = 1
    state = static_cast<uint16_t>(~gpio);

    // Ignore pins that are not connected to buttons.
    state &= ENABLED_PIN_MASK;

    return true;
}

ButtonLayout::Coord MCP23017Buttons::pinToCoord(
    uint8_t pin
) const
{
    // GPA0-GPA4 are physical pins 0-4.
    if (pin <= 4) {
        return {
            _firstColumn,
            pin
        };
    }

    // GPB0-GPB4 are physical pins 8-12.
    if (pin >= 8 && pin <= 12) {
        return {
            static_cast<uint8_t>(_firstColumn + 1),
            static_cast<uint8_t>(pin - 8)
        };
    }

    // Return the default invalid coordinate.
    return {};
}

ButtonEvents MCP23017Buttons::update()
{
    ButtonEvents events{};

    const uint32_t now = millis();

    if ((now - _lastPollMs) < POLL_MS) {
        return events;
    }

    _lastPollMs = now;

    uint16_t newRawState = 0;

    if (!readRawState(newRawState)) {
        return events;
    }

    /*
     * A raw state change restarts the debounce period.
     *
     * Example:
     *   first read:  button appears pressed
     *   next read:   button bounces back to released
     *
     * Each change updates _lastChangeMs. The state must remain unchanged
     * for DEBOUNCE_MS before it becomes the stable state.
     */
    if (newRawState != _rawState16) {
        _rawState16 = newRawState;
        _lastChangeMs = now;

        return events;
    }

    // The raw state has not yet remained unchanged long enough.
    if ((now - _lastChangeMs) < DEBOUNCE_MS) {
        return events;
    }

    /*
     * The raw state is now stable.
     *
     * If it differs from the previous stable state, determine which
     * buttons were pressed and released.
     */
    if (_stableState16 != _rawState16) {
        const uint16_t oldStableState = _stableState16;

        _stableState16 = _rawState16;

        const uint16_t pressedPins =
            static_cast<uint16_t>(
                _stableState16 & ~oldStableState
            );

        const uint16_t releasedPins =
            static_cast<uint16_t>(
                oldStableState & ~_stableState16
            );

        for (uint8_t pin = 0; pin < 16; ++pin) {
            const uint16_t pinBit =
                static_cast<uint16_t>(1U << pin);

            if ((ENABLED_PIN_MASK & pinBit) == 0) {
                continue;
            }

            if ((pressedPins & pinBit) != 0) {
                events.pressed = pinToCoord(pin);
            }

            if ((releasedPins & pinBit) != 0) {
                events.released = pinToCoord(pin);
            }

            if ((_stableState16 & pinBit) != 0) {
                events.current = pinToCoord(pin);
            }
        }

        return events;
    }

    /*
     * No new press or release occurred, but a button may still be held.
     *
     * This means events.held is returned repeatedly while the button
     * remains stably pressed.
     */
    for (uint8_t pin = 0; pin < 16; ++pin) {
        const uint16_t pinBit =
            static_cast<uint16_t>(1U << pin);

        if ((ENABLED_PIN_MASK & pinBit) == 0) {
            continue;
        }

        if ((_stableState16 & pinBit) != 0) {
            events.current = pinToCoord(pin);
        }
    }

    return events;
}