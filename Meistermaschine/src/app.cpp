#include "app.h"

App::App()
    : _audioPlayer(),
      _trackLibrary(),
      _display(),
      _buttons(),
      _volume(),
      _currentButtonId(ButtonLayout::NO_BUTTON),
      _currentVolume(20),
      _playing(false),
      _currentPlaylist(),
      _currentPlaylistIndex(0)
{
}

bool App::begin()
{
    _display.begin();
    _display.showMessage("AmbGen ready");

    pinMode(AudioPins::RESET, OUTPUT);
    pinMode(AudioPins::CS, OUTPUT);
    pinMode(AudioPins::DCS, OUTPUT);
    pinMode(AudioPins::CARDCS, OUTPUT);
    pinMode(AudioPins::DREQ, INPUT);

    digitalWrite(AudioPins::CS, HIGH);
    digitalWrite(AudioPins::DCS, HIGH);
    digitalWrite(AudioPins::CARDCS, HIGH);

    // VS1053 zunächst stilllegen
    digitalWrite(AudioPins::RESET, LOW);
    delay(10);

    // ZUERST SD / Registry
    if (!_trackLibrary.begin()) {
        _display.showMessage("SD failed");
        return false;
    }

    // VS1053 freigeben
    digitalWrite(AudioPins::RESET, HIGH);
    delay(10);

    if (!_audioPlayer.begin()) {
        _display.showMessage("VS1053 failed");
        return false;
    }

    if (!_buttons.begin()) {
        _display.showMessage("Buttons failed");
        return false;
    }

    _volume.begin();
    _audioPlayer.setVolume(_currentVolume);
    _audioPlayer.enableBackgroundPlayback();

    Serial.println(F("App ready"));
    return true;
}

void App::update(uint32_t now)
{
    (void)now;

    const ButtonEvents ev = _buttons.update();

    if (ev.valid) {
        handleButtonEvents(ev);
    }

    updateVolume();
    updatePlayback();

    // Debug: Heartbeat
    static uint32_t lastHeartbeat = 0;
    if (millis() - lastHeartbeat >= 1000) {
        lastHeartbeat = millis();
        Serial.println(F("alive"));
}
}

void App::handleButtonEvents(const ButtonEvents& ev)
{
    ButtonLayout::ButtonId buttonId = ButtonLayout::NO_BUTTON;

    if (ButtonLayout::singleSetBitToButtonId(ev.pressed, buttonId)) {
        requestButton(buttonId);
    }
}

void App::updateVolume()
{
    uint8_t newVolume = 0;

    if (_volume.readIfChanged(newVolume)) {
        _currentVolume = newVolume;
        _audioPlayer.setVolume(_currentVolume);
    }
}

void App::requestButton(ButtonLayout::ButtonId buttonId)
{
    if (!ButtonLayout::isValid(buttonId)) {
        return;
    }

        // --- NEU: Toggle-Verhalten ---
    if (_playing && buttonId == _currentButtonId) {
        Serial.println(F("Stopping playback"));

        _audioPlayer.stop();
        _playing = false;
        _currentButtonId = ButtonLayout::NO_BUTTON;

        _display.showMessage("Stopped");
        return;
    }

    _currentPlaylistIndex = 0;
    _currentButtonId = buttonId;

    if (!_trackLibrary.loadPlaylist(buttonId, _currentPlaylist)) {
        Serial.println(F("No playlist for button"));
        return;
    }

    if (_currentPlaylist.trackCount == 0) {
        Serial.println(F("Empty playlist"));
        return;
    }

    _currentPlaylistIndex = 0;
    _currentButtonId = buttonId;

    if (_audioPlayer.playFile(_currentPlaylist.tracks[_currentPlaylistIndex])) {
        _playing = true;

        _display.showTrackName(_currentPlaylist.tracks[_currentPlaylistIndex]);

        Serial.print(F("Button "));
        if (buttonId < 10) {
            Serial.print('0');
        }
        Serial.print(buttonId);
        Serial.print(F(" -> "));
        Serial.println(_currentPlaylist.tracks[_currentPlaylistIndex]);
    } else {
        _playing = false;
    }
}

void App::updatePlayback()
{
    if (!_playing) {
        return;
    }

    if (!_audioPlayer.isStopped()) {
        return;
    }

    ++_currentPlaylistIndex;

    if (_currentPlaylistIndex >= _currentPlaylist.trackCount) {
        _currentPlaylistIndex = 0;   // loop
    }

    if (_audioPlayer.playFile(_currentPlaylist.tracks[_currentPlaylistIndex])) {
        _display.showTrackName(_currentPlaylist.tracks[_currentPlaylistIndex]);

        Serial.print(F("Next track -> "));
        Serial.println(_currentPlaylist.tracks[_currentPlaylistIndex]);
    } else {
        _playing = false;
        Serial.println(F("Failed to start next track"));
    }
}