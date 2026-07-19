#ifndef APP_H
#define APP_H

#include <Arduino.h>
#include "AudioPlayer.h"
#include "TrackLibrary.h"
#include "DisplayService.h"
//#include "LocalButtons\LocalButtons.h"
#include "MCP23017Buttons\MCP23017Buttons.h"
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
    void handleButtonEvents(const ButtonEvents& ev);
    void updateVolume();
    void updatePlayback();

    void requestButton(ButtonLayout::ButtonId buttonId);
    //bool startCurrentTrack();

    inline bool hasActiveButton() const
    {
        return _currentButtonId != ButtonLayout::NO_BUTTON;
    }

    inline ButtonLayout::ButtonId currentButton() const
    {
        return _currentButtonId;
    }

private:
    AudioPlayer _audioPlayer;
    TrackLibrary _trackLibrary;
    DisplayService _display;
    //LocalButtons _buttons;
    MCP23017Driver _mcp1;
    MCP23017Driver _mcp2;
    MCP23017Buttons _buttons1;
    MCP23017Buttons _buttons2;
    VolumeService _volume;

    ButtonLayout::ButtonId _currentButtonId;
    uint8_t _currentVolume;
    bool _playing;

    TrackLibrary::Playlist _currentPlaylist;
    uint8_t _currentPlaylistIndex;
};

#endif