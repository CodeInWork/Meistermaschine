#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include "HardwareConfig.h"


class AudioPlayer
{
public:
    AudioPlayer();

    bool begin();
    void enableBackgroundPlayback();

    bool playFile(const char* fileName);
    void stop();
    void setVolume(uint8_t volume);

    bool isStopped();
    bool isPlaying();

private:
    Adafruit_VS1053_FilePlayer _player;
    uint8_t _currentVolume = 20;
};

#endif