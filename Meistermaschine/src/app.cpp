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

    if (!_audioPlayer.begin()) {
        _display.showMessage("VS1053 failed");
        return false;
    }

    if (!_trackLibrary.begin()) {
        _display.showMessage("SD failed");
        return false;
    }

    if (!_buttons.begin()) {
        _display.showMessage("Buttons failed");
        return false;
    }

    _volume.begin();
    _audioPlayer.setVolume(_currentVolume);

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
}

void App::updatePlayback()
{
    if (!_playing) {
        return;
    }

    if (!_audioPlayer.isStopped()) {
        return;
    }

    // aktueller Track ist beendet -> nächsten starten
    ++_currentPlaylistIndex;

    // aktuell: Playlist immer loopen
    if (_currentPlaylistIndex >= _currentPlaylist.trackCount) {
        _currentPlaylistIndex = 0;
    }

    if (!startCurrentTrack()) {
        _playing = false;
        _currentButtonId = ButtonLayout::NO_BUTTON;
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

    if (buttonId == _currentButtonId) {
        return;
    }

    // for debugging button IDs
    Serial.print(F("Requested button: "));
    Serial.println(buttonId);

    TrackLibrary::Playlist playlist;

    if (!_trackLibrary.loadPlaylist(buttonId, playlist)) {
        Serial.println(F("No playlist for button"));
        return;
    }

    if (playlist.trackCount == 0) {
        Serial.println(F("Empty playlist"));
        return;
    }

    _currentPlaylist = playlist;
    _currentPlaylistIndex = 0;
    _currentButtonId = buttonId;

    if (startCurrentTrack()) {
        _playing = true;

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

bool App::startCurrentTrack()
{
    if (_currentPlaylist.trackCount == 0) {
        return false;
    }

    if (_currentPlaylistIndex >= _currentPlaylist.trackCount) {
        return false;
    }

    const char* fileName = _currentPlaylist.tracks[_currentPlaylistIndex];

    if (!_audioPlayer.playFile(fileName)) {
        Serial.println(F("Failed to start track"));
        return false;
    }

    _display.showTrackName(fileName);
    return true;
}