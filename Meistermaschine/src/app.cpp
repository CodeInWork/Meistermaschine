#include "app.h"

namespace AppHw
{
    void initSpiChipSelects()
    {
        pinMode(AudioPins::CS, OUTPUT);
        pinMode(AudioPins::DCS, OUTPUT);
        pinMode(AudioPins::CARDCS, OUTPUT);
        pinMode(AudioPins::DREQ, INPUT);

        digitalWrite(AudioPins::CS, HIGH);
        digitalWrite(AudioPins::DCS, HIGH);
        digitalWrite(AudioPins::CARDCS, HIGH);
    }
}

App::App()
    : _audioPlayer(),
      _trackLibrary(),
      _display(),
      _buttons(),
      _volume(),
      _currentTrack(0),
      _currentVolume(20),
      _playing(false)
{
}

bool App::begin()
{
    AppHw::initSpiChipSelects();
    delay(10);          // only once after power-up, before the first SPI access, a delay is needed to let the VS1053 initialize properly

    _display.begin();
    _display.showMessage("AmbGen ready");

    if (!_audioPlayer.begin()) {
        _display.showMessage("VS1053 failed");
        return false;
    }

    if (!_trackLibrary.begin()) {
        _display.showMessage("SD failed");
        return false;
    }

    if (!_buttons.begin()) {
        _display.showMessage("Buttons failed");
        return false;
    }

    _volume.begin();
    _audioPlayer.setVolume(_currentVolume);

    return true;
}

void App::update(uint32_t now)
{
    (void)now;

    const ButtonEvents ev = _buttons.update();
    if (ev.valid) {
        handleButtonEvents(ev);
    }

    updateVolume();
}

void App::handleButtonEvents(const ButtonEvents& ev)
{
    if (ev.pressed & LocalButtonMask::BTN_1) {
        requestTrack(1);
    }

    if (ev.pressed & LocalButtonMask::BTN_2) {
        requestTrack(2);
    }
}

void App::updateVolume()
{
    uint8_t newVolume = 0;
    if (_volume.readIfChanged(newVolume)) {
        _currentVolume = newVolume;
        _audioPlayer.setVolume(_currentVolume);
    }
}

void App::requestTrack(uint8_t trackNumber)
{
    if (trackNumber == _currentTrack) {
        return;
    }

    char fileName[22] = {0};

    if (!_trackLibrary.getTrackFileName(trackNumber, fileName, sizeof(fileName))) {
        Serial.println("Track not found");
        return;
    }

    if (_audioPlayer.playFile(fileName)) {
        _currentTrack = trackNumber;
        _playing = true;
        _display.showTrackName(fileName);
    }
}