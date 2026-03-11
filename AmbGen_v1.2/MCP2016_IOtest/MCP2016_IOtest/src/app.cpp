#include "app.h"

App::App(MCP23017Driver& driver, MCP23017Buttons& buttons)
    : _driver(driver),
      _buttons(buttons),
      _ledState(0),
      _outputsDirty(true),
      _led1State(Led1State::Off),
      _lastBlinkMs(0) {}

bool App::begin()
{
    const uint16_t buttonMask =
        AppPins::BTN_0 |
        AppPins::BTN_1;

    const uint16_t ledMask =
        AppPins::LED_0 |
        AppPins::LED_1;

    _buttons.setButtonMask(buttonMask);

    uint16_t direction = 0xFFFF;

    direction &= ~ledMask;

    if(!_driver.setDirection(direction))
        return false;

    if(!_driver.setPullups(buttonMask))
        return false;

    _ledState = 0;
    _outputsDirty = true;

    updateOutputs();

    return true;
}

void App::update(uint32_t now)
{
    const ButtonEvents ev = _buttons.update();

    if (ev.valid) {
        handleButtonEvents(ev);
    }

    updateLed1State(now);
    updateOutputs();
}

void App::handleButtonEvents(const ButtonEvents& ev)
{
    if (ev.pressed & AppPins::BTN_0) {
        toggleLed(AppPins::LED_0);
        Serial.println("BTN_0 pressed -> toggle LED_0");
    }
    // state machine for LED_1: Off -> On -> Blinking -> Off -> ...
    if (ev.pressed & AppPins::BTN_1) {
        switch (_led1State) {
            case Led1State::Off:
                _led1State = Led1State::On;
                Serial.println("LED_1 state -> On");
                break;

            case Led1State::On:
                _led1State = Led1State::Blinking;
                _lastBlinkMs = millis();
                Serial.println("LED_1 state -> Blinking");
                break;

            case Led1State::Blinking:
                _led1State = Led1State::Off;
                Serial.println("LED_1 state -> Off");
                break;
        }
    }
}

void App::updateLed1State(uint32_t now)
{
    switch (_led1State) {
        case Led1State::Off:
            setLed(AppPins::LED_1, false);
            break;

        case Led1State::On:
            setLed(AppPins::LED_1, true);
            break;

        case Led1State::Blinking:
            if ((now - _lastBlinkMs) >= BLINK_INTERVAL_MS) {
                _lastBlinkMs = now;
                toggleLed(AppPins::LED_1);
            }
            break;
    }
}

void App::updateOutputs()
{
    if (!_outputsDirty) {
        return;
    }

    if (_driver.setOutputState(_ledState)) {
        _outputsDirty = false;
    } else {
        Serial.println("Output write failed");
    }
}

void App::setLed(uint16_t ledMask, bool on)
{
    const uint16_t oldState = _ledState;

    if (on) {
        _ledState |= ledMask;
    } else {
        _ledState &= static_cast<uint16_t>(~ledMask);
    }

    if (_ledState != oldState) {
        _outputsDirty = true;
    }
}

void App::toggleLed(uint16_t ledMask)
{
    _ledState ^= ledMask;
    _outputsDirty = true;
}