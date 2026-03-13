#ifndef TRACK_LIBRARY_H
#define TRACK_LIBRARY_H

#include <Arduino.h>
#include <SD.h>

class TrackLibrary
{
public:
    TrackLibrary();

    bool begin();
    bool getTrackFileName(uint8_t trackNumber, char* outName, size_t outSize);

private:
    uint8_t countDigits(uint8_t number) const;
    bool matchTrackPrefix(const char* fileName, uint8_t trackNumber) const;

private:
    File _root;
};

#endif