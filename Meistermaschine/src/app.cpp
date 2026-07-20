#include "app.h"

App::App()
    : _audioPlayer(),
      _trackLibrary(),
      _display(),
      _mcp1(I2CAddresses::MCP_1),
      _mcp2(I2CAddresses::MCP_2),
      _buttons1(_mcp1, 0),
      _buttons2(_mcp2, 2),
      _volume(),
      _currentButton(),
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

    if (!_buttons1.begin()) {
        _display.showMessage("MCP1 buttons failed");
        return false;
    }

    if (!_buttons2.begin()) {
        _display.showMessage("MCP2 buttons failed");
        return false;
    }

    _volume.begin();
    _audioPlayer.setVolume(_currentVolume);
    // interrupt mode proved unstable on Arduino Nano Every -> moved to cooperative feeding
    //_audioPlayer.enableBackgroundPlayback();

    Serial.println(F("App ready"));
    return true;
}

void App::update(uint32_t now)
{
    // Cooperative feeding of the audio player.
    _audioPlayer.update();

    const ButtonEvents events1 = _buttons1.update();
    const ButtonEvents events2 = _buttons2.update();

    ButtonEvents combinedEvents{};

    /*
     * When both MCPs report a transition in the same App cycle,
     * the event from MCP2 overwrites the event from MCP1.
     */
    if (ButtonLayout::isValid(events1.pressed)) {
        combinedEvents.pressed = events1.pressed;
    }

    if (ButtonLayout::isValid(events2.pressed)) {
        combinedEvents.pressed = events2.pressed;
    }

    if (ButtonLayout::isValid(events1.released)) {
        combinedEvents.released = events1.released;
    }

    if (ButtonLayout::isValid(events2.released)) {
        combinedEvents.released = events2.released;
    }

    /*
     * current describes a button that is currently down.
     * MCP2 receives priority only when it actually has a current button.
     */
    if (ButtonLayout::isValid(events1.current)) {
        combinedEvents.current = events1.current;
    }

    if (ButtonLayout::isValid(events2.current)) {
        combinedEvents.current = events2.current;
    }

    const bool hasButtonEvent =
        ButtonLayout::isValid(combinedEvents.pressed) ||
        ButtonLayout::isValid(combinedEvents.released) ||
        ButtonLayout::isValid(combinedEvents.current);

    if (hasButtonEvent) {
        handleButtonEvents(combinedEvents);
    }

    updateVolume();
    updatePlayback();
}

void App::handleButtonEvents(const ButtonEvents& events)
{
    if (ButtonLayout::isValid(events.pressed)) {
        requestButton(events.pressed);

        Serial.print(F("Pressed: column "));
        Serial.print(events.pressed.column);
        Serial.print(F(", row "));
        Serial.println(events.pressed.row);
    }

    if (ButtonLayout::isValid(events.released)) {
        Serial.print(F("Released: column "));
        Serial.print(events.released.column);
        Serial.print(F(", row "));
        Serial.println(events.released.row);
    }
}

bool App::hasActiveButton() const
{
    return ButtonLayout::isValid(_currentButton);
}

bool App::isCurrentButton(
    const ButtonLayout::Coord& button
) const
{
    return
        hasActiveButton() &&
        _currentButton.column == button.column &&
        _currentButton.row == button.row;
}

void App::updateVolume()
{
    uint8_t newVolume = 0;

    if (_volume.readIfChanged(newVolume)) {
        _currentVolume = newVolume;
        _audioPlayer.setVolume(_currentVolume);
    }
}

void App::requestButton(
    const ButtonLayout::Coord& button
)
{
    if (!ButtonLayout::isValid(button)) {
        return;
    }

    // Toggle: pressing the currently active button stops playback.
    if (_playing && isCurrentButton(button)) {
        Serial.println(F("Stopping playback"));

        _audioPlayer.stop();
        _playing = false;
        _currentButton = ButtonLayout::Coord{};

        _display.showMessage("Stopped");
        return;
    }

    _currentPlaylistIndex = 0;
    _currentButton = button;

    if (!_trackLibrary.loadPlaylist(button, _currentPlaylist)) {
        Serial.println(F("No playlist for button"));
        _currentButton = ButtonLayout::Coord{};
        return;
    }

    if (_currentPlaylist.trackCount == 0) {
        Serial.println(F("Empty playlist"));
        _currentButton = ButtonLayout::Coord{};
        return;
    }

    if (_audioPlayer.playFile(
            _currentPlaylist.tracks[_currentPlaylistIndex])) {
        _playing = true;

        _display.showTrackName(
            _currentPlaylist.tracks[_currentPlaylistIndex]);

        Serial.print(F("Button ["));
        Serial.print(button.column);
        Serial.print(F("]["));
        Serial.print(button.row);
        Serial.print(F("] -> "));
        Serial.println(
            _currentPlaylist.tracks[_currentPlaylistIndex]);
    } else {
        _playing = false;
        _currentButton = ButtonLayout::Coord{};
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