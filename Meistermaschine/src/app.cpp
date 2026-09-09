#include "app.h"
#include "SoftwareConfig.h"

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
      _previewActive(false),
      _currentPlaylist(),
      _currentPlaylistIndex(0),
      _pendingClickButton(),
      _pendingClickTimeMs(0),
      _clickPending(false)
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

    if (SoftwareConfig::DEBUG) {
        Serial.println(F("App ready"));
    }
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

    if (ButtonLayout::isValid(events1.held)) {
        combinedEvents.held = events1.held;
        combinedEvents.pressDurationMs = events1.pressDurationMs;
    }

    if (ButtonLayout::isValid(events2.held)) {
        combinedEvents.held = events2.held;
        combinedEvents.pressDurationMs = events2.pressDurationMs;
    }

    if (ButtonLayout::isValid(events1.released)) {
        combinedEvents.released = events1.released;
        combinedEvents.pressDurationMs = events1.pressDurationMs;
    }

    if (ButtonLayout::isValid(events2.released)) {
        combinedEvents.released = events2.released;
        combinedEvents.pressDurationMs = events2.pressDurationMs;
    }

    const bool hasButtonEvent =
        ButtonLayout::isValid(combinedEvents.held) ||
        ButtonLayout::isValid(combinedEvents.released);

    if (hasButtonEvent) {
        handleButtonEvents(combinedEvents, now);
    }

    updatePendingClick(now);

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
    if (_previewActive) {return;}

    _display.showMessage(_trackLibrary.presetName());
    _displayState = DisplayState::Preset;
}

void App::showCurrentTrack()
{
    if (_previewActive) {
        return;
    }
    
    if (
        !_playing ||
        !ButtonLayout::isValid(_currentButton) ||
        _currentPlaylistIndex >= _currentPlaylist.trackCount
    ) {
        return;
    }

    char buttonText[8];

    setButtonText(buttonText, sizeof(buttonText), _currentButton);

    _display.showLayout(
        _currentPlaylist.titles[_currentPlaylistIndex],
        buttonText,
        ""
    );
}

void App::handleButtonEvents(
    const ButtonEvents& events,
    uint32_t now
)
{
    // Long press / preview
    if (
        ButtonLayout::isValid(events.held) &&
        events.pressDurationMs >= LONG_PRESS_MS &&
        !_previewActive
    ) {
        previewButton(events.held);
        return;
    }

    if (!ButtonLayout::isValid(events.released)) {
        return;
    }

    // Releasing a long press only closes the preview.
    if (_previewActive) {
        _previewActive = false;
        restoreDisplay();
        return;
    }

    /*
     * Second short click on the same button:
     * this is a double click.
     */
    if (
        _clickPending &&
        isSameButton(
            events.released,
            _pendingClickButton
        ) &&
        (now - _pendingClickTimeMs) <= DOUBLE_CLICK_MS
    ) {
        _clickPending = false;
        _pendingClickButton = ButtonLayout::Coord{};

        handleDoubleClick(events.released);
        return;
    }

    /*
     * First short click.
     * Do not execute it yet because a second click may follow.
     */
    _pendingClickButton = events.released;
    _pendingClickTimeMs = now;
    _clickPending = true;
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

void App::requestButton(const ButtonLayout::Coord& button)
{
    if (!ButtonLayout::isValid(button)) {
        return;
    }

    // Pressing the currently active button toggles playback off.
    if (_playing && isCurrentButton(button)) {
        if (SoftwareConfig::DEBUG) {
            Serial.println(F("Stopping playback"));
        }

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
        if (SoftwareConfig::DEBUG) {
            Serial.println(F("No playlist for button"));
        }
        return;
    }

    if (requestedPlaylist.trackCount == 0) {
        if (SoftwareConfig::DEBUG) {
            Serial.println(F("Empty playlist"));
        }
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

        if (SoftwareConfig::DEBUG) {
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
        }
    } else {
        if (SoftwareConfig::DEBUG) {
            Serial.println(F("Failed to start requested track"));
        }

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

    if (playNextTrack()) {
        return;
    }

    finishPlayback();
}

void App::clearCurrentPlaylist()
{
    _currentPlaylist = TrackLibrary::Playlist{};
}

void App::restoreDisplay()
{
    if (
        _playing &&
        ButtonLayout::isValid(_currentButton) &&
        _currentPlaylistIndex < _currentPlaylist.trackCount
    ) {
        showCurrentTrack();
    } else {
        showPresetName();
    }
}

void App::previewButton(const ButtonLayout::Coord& button)
{
    if (!ButtonLayout::isValid(button)) {
        return;
    }

    TrackLibrary::Playlist previewPlaylist;

    if (
        !_trackLibrary.loadPlaylist(
            button,
            previewPlaylist
        )
    ) {
        return;
    }

    if (previewPlaylist.trackCount == 0) {
        return;
    }

    char buttonText[8];

    setButtonText(buttonText, sizeof(buttonText), button);

    _previewActive = true;

    _display.showLayout(
        previewPlaylist.titles[0],
        buttonText,
        ""
    );
    if (SoftwareConfig::DEBUG) {
        Serial.print(F("Preview: "));
        Serial.println(previewPlaylist.titles[0]);
    }
}

bool App::playNextTrack()
{
    if (!_playing || _currentPlaylist.trackCount == 0) {
        return false;
    }

    ++_currentPlaylistIndex;

    if (_currentPlaylistIndex >= _currentPlaylist.trackCount) {
        if (!ButtonLayout::loops(_currentButton)) {
            return false;
        }

        _currentPlaylistIndex = 0;
    }

    if (!_audioPlayer.playFile(
        _currentPlaylist.tracks[_currentPlaylistIndex]
    )) {
        return false;
    }

    showCurrentTrack();

    if (SoftwareConfig::DEBUG) {
        Serial.print(F("Next track -> "));
        Serial.println(
            _currentPlaylist.tracks[_currentPlaylistIndex]
        );
    }

    return true;
}

bool App::isSameButton(
    const ButtonLayout::Coord& a,
    const ButtonLayout::Coord& b
) const
{
    return
        ButtonLayout::isValid(a) &&
        ButtonLayout::isValid(b) &&
        a.column == b.column &&
        a.row == b.row;
}

void App::updatePendingClick(uint32_t now)
{
    if (!_clickPending) {
        return;
    }

    if ((now - _pendingClickTimeMs) <= DOUBLE_CLICK_MS) {
        return;
    }

    const ButtonLayout::Coord button =
        _pendingClickButton;

    _clickPending = false;
    _pendingClickButton = ButtonLayout::Coord{};

    requestButton(button);
}

void App::handleDoubleClick(
    const ButtonLayout::Coord& button
)
{
    if (
        !_playing ||
        !isCurrentButton(button)
    ) {
        requestButton(button);
        return;
    }

    _audioPlayer.stop();

    if (!playNextTrack()) {
        finishPlayback();
    }
}

void App::finishPlayback()
{
    _playing = false;
    _currentButton = ButtonLayout::Coord{};
    _currentPlaylistIndex = 0;

    clearCurrentPlaylist();
    showPresetName();

    if (SoftwareConfig::DEBUG) {
        Serial.println(F("Playlist finished"));
    }
}

void App::setButtonText(char* buttonText, const size_t buttonTextSize, const ButtonLayout::Coord& button) const
{
    snprintf(
        buttonText,
        buttonTextSize,
        "C%u,R%u",
        button.column + 1,
        button.row + 1
    );
}