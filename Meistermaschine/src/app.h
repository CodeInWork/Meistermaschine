#ifndef APP_H
#define APP_H

#include <Arduino.h>
#include "AudioPlayer.h"
#include "TrackLibrary.h"
#include "DisplayService.h"
#include "LocalButtons.h"
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
    void requestTrack(uint8_t trackNumber);

private:
    AudioPlayer _audioPlayer;
    TrackLibrary _trackLibrary;
    DisplayService _display;
    LocalButtons _buttons;
    VolumeService _volume;

    uint8_t _currentTrack;
    uint8_t _currentVolume;
    bool _playing;
};

#endif