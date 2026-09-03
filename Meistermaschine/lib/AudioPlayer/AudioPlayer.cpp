#include "AudioPlayer.h"
#include "SoftwareConfig.h"

AudioPlayer::AudioPlayer()
    : _player(AudioPins::RESET,
              AudioPins::CS,
              AudioPins::DCS,
              AudioPins::DREQ,
              AudioPins::CARDCS)
{
}

bool AudioPlayer::begin()
{
    pinMode(AudioPins::CS, OUTPUT);
    pinMode(AudioPins::DCS, OUTPUT);
    pinMode(AudioPins::CARDCS, OUTPUT);
    pinMode(AudioPins::DREQ, INPUT);

    digitalWrite(AudioPins::CS, HIGH);
    digitalWrite(AudioPins::DCS, HIGH);
    digitalWrite(AudioPins::CARDCS, HIGH);

    if (!_player.begin()) {
        if (SoftwareConfig::DEBUG) {
            Serial.println(F("VS1053 not found"));
        }
        return false;
    }

    if (SoftwareConfig::DEBUG) {
        Serial.println(F("VS1053 found"));
    }
    _player.setVolume(_currentVolume, _currentVolume);

    return true;
}

void AudioPlayer::update()
{
    if (!_player.stopped()) {
        _player.feedBuffer();
    }
}

void AudioPlayer::enableBackgroundPlayback()
{
    _player.useInterrupt(VS1053_FILEPLAYER_PIN_INT);
}

bool AudioPlayer::playFile(const char* fileName)
{
    if (!fileName) {
        return false;
    }

    _player.stopPlaying();

    if (SoftwareConfig::DEBUG) {
        Serial.print(F("Playing: "));
        Serial.println(fileName);
    }


    return _player.startPlayingFile(fileName);
}

void AudioPlayer::stop()
{
    _player.stopPlaying();
}

void AudioPlayer::setVolume(uint8_t volume)
{
    _currentVolume = volume;
    _player.setVolume(volume, volume);
}

bool AudioPlayer::isStopped()
{
    return _player.stopped();
}

bool AudioPlayer::isPlaying()
{
    return !_player.stopped();
}