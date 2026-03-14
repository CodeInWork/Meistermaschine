#ifndef APP_H
#define APP_H

#include <Arduino.h>
#include "AudioPlayer.h"
#include "TrackLibrary.h"
#include "DisplayService.h"
#include "LocalButtons/LocalButtons.h"
#include "VolumeService.h"

class App
{
public:
    App();

    bool begin();
    void update(uint32_t now);

private:
    void handleButtonEvents(const ButtonEvents& ev);
    void updateVolume();
    void requestButton(uint8_t buttonId);
    inline bool hasActiveButton() const
    {
        return _currentButtonId != 255;
    }
    inline uint8_t currentButton() const
    {
        return _currentButtonId;
    }

private:
    AudioPlayer _audioPlayer;
    TrackLibrary _trackLibrary;
    DisplayService _display;
    LocalButtons _buttons;
    VolumeService _volume;

    uint8_t _currentButtonId;
    uint8_t _currentVolume;
    bool _playing;
};

#endif