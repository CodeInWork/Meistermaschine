#include "MCP23017Buttons.h"

// Constructor for button operations
MCP23017Buttons::MCP23017Buttons(MCP23017Driver& driver,
                                 uint32_t pollIntervalMs,
                                 uint32_t debounceTimeMs)
    : _driver(driver),
      _pollIntervalMs(pollIntervalMs),
      _debounceTimeMs(debounceTimeMs),
      _rawState(0xFFFF),                // pull-Ups active -> all HIGH = unpressed (1111111111111111)
      _stableState(0xFFFF),             // for polling. Initially also (11111111111111111) = all unpressed
      _lastPollMs(0),
      _lastChangeMs(0) {}            // flag to indicate if begin() was successfully called

void MCP23017Buttons::setButtonMask(uint16_t mask)
{
    _buttonMask = mask;
}

ButtonEvents MCP23017Buttons::update() {
    ButtonEvents events{};

    const uint32_t now = millis();

    if ((now - _lastPollMs) < _pollIntervalMs) {
        return events;
    }

    _lastPollMs = now;

    uint16_t newRaw = 0;
    
    if (!_driver.readGPIO(newRaw)) {
        return events;
    }

    // Nicht benutzte Tasterleitungen immer auf HIGH setzen
    newRaw |= static_cast<uint16_t>(~_buttonMask);

    if (newRaw != _rawState) {
        _rawState = newRaw;
        _lastChangeMs = now;
    }

    if ((now - _lastChangeMs) < _debounceTimeMs) {
        return events;
    }

    if (_stableState != _rawState) {
        const uint16_t oldStable = _stableState;
        const uint16_t newStable = _rawState;

        _stableState = newStable;

        // Pull-Up-Logik:
        // HIGH = offen
        // LOW  = gedrückt
        const uint16_t oldPressed = static_cast<uint16_t>(~oldStable) & _buttonMask;
        const uint16_t newPressed = static_cast<uint16_t>(~newStable) & _buttonMask;

        events.pressed  = static_cast<uint16_t>( newPressed & ~oldPressed);
        events.released = static_cast<uint16_t>( oldPressed & ~newPressed);
        events.held     = newPressed;
        events.valid = true;
    } else {
        events.held = static_cast<uint16_t>(~_stableState) & _buttonMask;
        events.valid = true;
    }

    return events;
}
