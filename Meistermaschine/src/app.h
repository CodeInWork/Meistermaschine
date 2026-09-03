#ifndef APP_H
#define APP_H

#include <Arduino.h>

#include "AudioPlayer.h"
#include "TrackLibrary.h"
#include "DisplayService.h"
#include "MCP23017Buttons/MCP23017Buttons.h"
#include "MCP23017Driver.h"
#include "VolumeService.h"
#include "ButtonUtils.h"
#include "ButtonEvents.h"

class App
{
public:
    App();

    bool begin();
    void update(uint32_t now);

private:
    void handleButtonEvents(const ButtonEvents& events);
    void updateVolume();
    void updatePlayback();
    void updateDisplay(uint32_t now);

    void showPresetName();
    void showCurrentTrack();
    void restoreDisplay();
    void clearCurrentPlaylist();

    void previewButton(const ButtonLayout::Coord& button);
    void requestButton(const ButtonLayout::Coord& button);

    bool hasActiveButton() const;
    bool isCurrentButton(const ButtonLayout::Coord& button) const;

private:
    static constexpr uint32_t LONG_PRESS_MS = 1000; // defines time threshold for a long press event

    enum class DisplayState
    {
        Startup,
        Preset,
        Track
    };
    DisplayState _displayState;
    uint32_t _displayStateStartedMs;

    AudioPlayer _audioPlayer;
    TrackLibrary _trackLibrary;
    DisplayService _display;

    MCP23017Driver _mcp1;
    MCP23017Driver _mcp2;
    MCP23017Buttons _buttons1;
    MCP23017Buttons _buttons2;

    VolumeService _volume;

    ButtonLayout::Coord _currentButton;
    uint8_t _currentVolume;
    bool _playing;
    bool _previewActive;

    TrackLibrary::Playlist _currentPlaylist;
    uint8_t _currentPlaylistIndex;
};

#endif