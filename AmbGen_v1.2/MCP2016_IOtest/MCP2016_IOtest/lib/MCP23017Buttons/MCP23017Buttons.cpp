#include "MCP23017Buttons.h"

// Constructor for button operations
MCP23017Buttons::MCP23017Buttons(MCP23017Driver& driver,
                                 uint32_t pollIntervalMs,
                                 uint32_t debounceTimeMs)
    : _driver(driver),
      _enabledMask(Buttons::ALL),       // default: all buttons enabled
      _pollIntervalMs(pollIntervalMs),
      _debounceTimeMs(debounceTimeMs),
      _rawState(0xFFFF),                // pull-Ups active -> all HIGH = unpressed (1111111111111111)
      _stableState(0xFFFF),             // for polling. Initially also (11111111111111111) = all unpressed
      _lastPollMs(0),
      _lastChangeMs(0),
      _initialized(false) {}            // flag to indicate if begin() was successfully called

bool MCP23017Buttons::begin(uint16_t enabledMask) {
    _enabledMask = enabledMask;

    // alle Pins erstmal Input
    if (!_driver.setDirection(0xFFFF)) {
        return false;
    }

    // Pull-Ups nur auf den aktiv genutzten Tasterpins
    if (!_driver.setPullups(_enabledMask)) {
        return false;
    }

    uint16_t initialState = 0;
    if (!_driver.readGPIO(initialState)) {
        return false;
    }

    // Nicht genutzte Pins logisch auf HIGH maskieren,
    // damit sie nie als "gedrückt" erscheinen.
    initialState |= static_cast<uint16_t>(~_enabledMask);

    _rawState = initialState;
    _stableState = initialState;
    _lastPollMs = millis();
    _lastChangeMs = _lastPollMs;
    _initialized = true;

    return true;
}

ButtonEvents MCP23017Buttons::update() {
    ButtonEvents events{};

    if (!_initialized) {
        return events;
    }

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
    newRaw |= static_cast<uint16_t>(~_enabledMask);

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
        const uint16_t oldPressed = static_cast<uint16_t>(~oldStable) & _enabledMask;
        const uint16_t newPressed = static_cast<uint16_t>(~newStable) & _enabledMask;

        events.pressed  = static_cast<uint16_t>( newPressed & ~oldPressed);
        events.released = static_cast<uint16_t>( oldPressed & ~newPressed);
        events.held     = newPressed;
        events.valid = true;
    } else {
        events.held = static_cast<uint16_t>(~_stableState) & _enabledMask;
        events.valid = true;
    }

    return events;
}

uint16_t MCP23017Buttons::rawState() const {
    return _rawState;
}

uint16_t MCP23017Buttons::stableState() const {
    return _stableState;
}

uint16_t MCP23017Buttons::pressedState() const {
    return static_cast<uint16_t>(~_stableState) & _enabledMask;
}