#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>

namespace AudioPins {
    constexpr uint8_t RESET = 9;
    constexpr uint8_t CS    = 10;
    constexpr uint8_t DCS   = 8;
    constexpr uint8_t CARDCS = 4;
    constexpr uint8_t DREQ   = 3;
}

class AudioPlayer
{
public:
    AudioPlayer();

    bool begin();
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