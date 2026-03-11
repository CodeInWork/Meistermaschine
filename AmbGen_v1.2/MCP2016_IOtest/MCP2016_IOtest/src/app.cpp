#include "app.h"

App::App(MCP23017Driver& driver, MCP23017Buttons& buttons)
    : _driver(driver),
      _buttons(buttons),
      _ledState(0),
      _outputsDirty(true),
      _appState(AppState::Idle),
      _led1State(Led1State::Off),
      _lastBlinkMs(0)
{
}

bool App::begin()
{
    const uint16_t buttonMask =
        AppPins::BTN_0 |
        AppPins::BTN_1;

    const uint16_t ledMask =
        AppPins::LED_0 |
        AppPins::LED_1;

    _buttons.setButtonMask(buttonMask);

    // Alle Pins zunächst als Input
    uint16_t directionMask = 0xFFFF;

    // LED-Pins als Output
    directionMask &= static_cast<uint16_t>(~ledMask);

    if (!_driver.setDirection(directionMask)) {
        return false;
    }

    // Pull-Ups nur für Taster-Pins
    if (!_driver.setPullups(buttonMask)) {
        return false;
    }

    _ledState = 0;
    _outputsDirty = true;
    _appState = AppState::Idle;
    _led1State = Led1State::Off;
    _lastBlinkMs = 0;

    updateOutputs();

    return true;
}

void App::update(uint32_t now)
{
    const ButtonEvents ev = _buttons.update();

    if (ev.valid) {
        handleButtonEvents(ev, now);
    }

    updateAppState(now);
    updateOutputs();
}

void App::handleButtonEvents(const ButtonEvents& ev, uint32_t now)
{
    // BTN_0 schaltet zwischen Idle und Active um
    if (ev.pressed & AppPins::BTN_0) {
        switch (_appState) {
            case AppState::Idle:
                _appState = AppState::Active;
                Serial.println("AppState -> Active");
                break;

            case AppState::Active:
                _appState = AppState::Idle;
                Serial.println("AppState -> Idle");
                break;

            case AppState::Error:
                // optional: aus Error heraus resetten
                _appState = AppState::Idle;
                _led1State = Led1State::Off;
                Serial.println("AppState -> Idle (from Error)");
                break;
        }
    }

    // BTN_1 steuert LED_1-Zustände, aber nur wenn wir nicht im Error sind
    if ((ev.pressed & AppPins::BTN_1) && (_appState != AppState::Error)) {
        switch (_led1State) {
            case Led1State::Off:
                _led1State = Led1State::On;
                Serial.println("LED_1 state -> On");
                break;

            case Led1State::On:
                _led1State = Led1State::Blinking;
                _lastBlinkMs = now;
                setLed(AppPins::LED_1, true);
                Serial.println("LED_1 state -> Blinking");
                break;

            case Led1State::Blinking:
                _led1State = Led1State::Off;
                Serial.println("LED_1 state -> Off");
                break;
        }
    }
}

void App::updateAppState(uint32_t now)
{
    switch (_appState) {
        case AppState::Idle:
            updateIdleState(now);
            break;

        case AppState::Active:
            updateActiveState(now);
            break;

        case AppState::Error:
            updateErrorState(now);
            break;
    }
}

void App::updateIdleState(uint32_t now)
{
    (void)now;

    // In Idle ist LED_0 aus
    setLed(AppPins::LED_0, false);

    // LED_1 folgt ihrer eigenen kleinen State Machine
    updateLed1State(now);
}

void App::updateActiveState(uint32_t now)
{
    // In Active ist LED_0 an
    setLed(AppPins::LED_0, true);

    // LED_1 folgt ebenfalls ihrer State Machine
    updateLed1State(now);
}

void App::updateErrorState(uint32_t now)
{
    (void)now;

    // Im Fehlerfall alles aus
    setLed(AppPins::LED_0, false);
    setLed(AppPins::LED_1, false);
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
        _appState = AppState::Error;
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