#include "AudioPlayer.h"

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
    // SPI Teilnehmer sicher deselektieren
    pinMode(AudioPins::CS, OUTPUT);
    pinMode(AudioPins::DCS, OUTPUT);
    pinMode(AudioPins::CARDCS, OUTPUT);
    pinMode(AudioPins::DREQ, INPUT);

    digitalWrite(AudioPins::CS, HIGH);
    digitalWrite(AudioPins::DCS, HIGH);
    digitalWrite(AudioPins::CARDCS, HIGH);

    // VS1053 initialisieren
    if (!_player.begin()) {
        Serial.println(F("VS1053 not found"));
        return false;
    }

    Serial.println(F("VS1053 found"));

    // Interruptmodus aktivieren (für Hintergrund-Playback)
    _player.useInterrupt(VS1053_FILEPLAYER_PIN_INT);

    // Startlautstärke
    _player.setVolume(_currentVolume, _currentVolume);

    return true;
}

bool AudioPlayer::playFile(const char* fileName)
{
    if (!fileName) {
        return false;
    }

    _player.stopPlaying();

    Serial.print(F("Playing: "));
    Serial.println(fileName);

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