#ifndef TRACK_LIBRARY_H
#define TRACK_LIBRARY_H

#include <Arduino.h>
#include <SD.h>

class TrackLibrary
{
public:
    static constexpr const char* REGISTRY_FILE = "REG.MMS";
    static constexpr uint8_t MAX_PLAYLIST_TRACKS = 8;
    static constexpr uint8_t BUTTON_ID_LEN = 2;
    static constexpr uint8_t MAX_FILENAME_LEN = 24;
    static constexpr uint8_t MAX_LINE_LEN = MAX_FILENAME_LEN + BUTTON_ID_LEN + 2; // 1 for separator, 1 for null terminator

    struct Playlist
    {
        uint8_t buttonId = 0;
        uint8_t trackCount = 0;
        char tracks[MAX_PLAYLIST_TRACKS][MAX_FILENAME_LEN] = {{0}};
    };

public:
    TrackLibrary();

    bool begin();
    bool loadPlaylist(uint8_t buttonId, Playlist& playlist) const;
    bool hasTrack(uint8_t buttonId) const;

private:
    bool parseLine(const char* line, uint8_t& buttonId, char* fileName, size_t fileNameSize) const;
    bool isDigit(char c) const;
    void clearPlaylist(Playlist& playlist) const;

private:
    bool _initialized;
};

#endif