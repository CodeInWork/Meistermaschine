#include "app.h"

App::App()
    : _displayState(DisplayState::Startup),
      _displayStateStartedMs(0),
      _audioPlayer(),
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
    _display.showMessage("MEISTERMASCHINE");

    _displayState = DisplayState::Startup;
    _displayStateStartedMs = millis();

    pinMode(AudioPins::RESET, OUTPUT);
    pinMode(AudioPins::CS, OUTPUT);
    pinMode(AudioPins::DCS, OUTPUT);
    pinMode(AudioPins::CARDCS, OUTPUT);
    pinMode(AudioPins::DREQ, INPUT);

    digitalWrite(AudioPins::CS, HIGH);
    digitalWrite(AudioPins::DCS, HIGH);
    digitalWrite(AudioPins::CARDCS, HIGH);

    // Keep VS1053 inactive while initializing the SD card.
    digitalWrite(AudioPins::RESET, LOW);
    delay(10);

    if (!_trackLibrary.begin()) {
        _display.showMessage("SD Fehler");
        return false;
    }

    digitalWrite(AudioPins::RESET, HIGH);
    delay(10);

    if (!_audioPlayer.begin()) {
        _display.showMessage("VS1053 Fehler");
        return false;
    }

    if (!_buttons1.begin()) {
        _display.showMessage("MCP1 Fehler");
        return false;
    }

    if (!_buttons2.begin()) {
        _display.showMessage("MCP2 Fehler");
        return false;
    }

    _volume.begin();
    _audioPlayer.setVolume(_currentVolume);

    Serial.println(F("App ready"));
    return true;
}

void App::update(uint32_t now)
{
    updateDisplay(now);

    // Cooperative feeding of the audio player.
    _audioPlayer.update();

    const ButtonEvents events1 = _buttons1.update();
    const ButtonEvents events2 = _buttons2.update();

    ButtonEvents combinedEvents{};

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

void App::updateDisplay(uint32_t now)
{
    static constexpr uint32_t STARTUP_DISPLAY_MS = 2000;

    if (
        _displayState == DisplayState::Startup &&
        (now - _displayStateStartedMs) >= STARTUP_DISPLAY_MS
    ) {
        showPresetName();
    }
}

void App::showPresetName()
{
    _display.showMessage(_trackLibrary.presetName());
    _displayState = DisplayState::Preset;
}

void App::showCurrentTrack()
{
    if (
        !_playing ||
        !ButtonLayout::isValid(_currentButton) ||
        _currentPlaylistIndex >= _currentPlaylist.trackCount
    ) {
        return;
    }

    char buttonText[8];

    snprintf(
        buttonText,
        sizeof(buttonText),
        "BTN %u,%u",
        _currentButton.column+1,
        _currentButton.row+1
    );

    _display.showLayout(
        _currentPlaylist.titles[_currentPlaylistIndex],
        buttonText,
        ""
    );
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

    // Pressing the currently active button toggles playback off.
    if (_playing && isCurrentButton(button)) {
        Serial.println(F("Stopping playback"));

        _audioPlayer.stop();

        _playing = false;
        _currentButton = ButtonLayout::Coord{};
        _currentPlaylistIndex = 0;
        clearCurrentPlaylist();

        showPresetName();
        return;
    }

    /*
     * First load the requested playlist into a temporary object.
     * Do not modify the active playback state yet.
     */
    TrackLibrary::Playlist requestedPlaylist;

    if (
        !_trackLibrary.loadPlaylist(
            button,
            requestedPlaylist
        )
    ) {
        Serial.println(F("No playlist for button"));
        return;
    }

    if (requestedPlaylist.trackCount == 0) {
        Serial.println(F("Empty playlist"));
        return;
    }

    /*
     * The new playlist is valid. We can now stop the old track
     * and commit the new selection.
     */
    if (_playing) {
        _audioPlayer.stop();
    }

    _currentPlaylist = requestedPlaylist;
    _currentPlaylistIndex = 0;
    _currentButton = button;

    if (
        _audioPlayer.playFile(
            _currentPlaylist.tracks[
                _currentPlaylistIndex
            ]
        )
    ) {
        _playing = true;
        showCurrentTrack();

        Serial.print(F("Button ["));
        Serial.print(button.column);
        Serial.print(F("]["));
        Serial.print(button.row);
        Serial.print(F("] -> "));
        Serial.println(
            _currentPlaylist.tracks[
                _currentPlaylistIndex
            ]
        );
    } else {
        Serial.println(F("Failed to start requested track"));

        _playing = false;
        _currentButton = ButtonLayout::Coord{};
        _currentPlaylistIndex = 0;
        clearCurrentPlaylist();

        showPresetName();
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
        _currentPlaylistIndex = 0;
    }

    if (
        _audioPlayer.playFile(
            _currentPlaylist.tracks[_currentPlaylistIndex]
        )
    ) {
        showCurrentTrack();

        Serial.print(F("Next track -> "));
        Serial.println(
            _currentPlaylist.tracks[_currentPlaylistIndex]
        );
    } else {
        _playing = false;
        _currentButton = ButtonLayout::Coord{};

        showPresetName();

        Serial.println(F("Failed to start next track"));
    }
}

void App::clearCurrentPlaylist()
{
    _currentPlaylist = TrackLibrary::Playlist{};
}