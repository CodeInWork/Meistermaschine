#ifndef TRACK_LIBRARY_H
#define TRACK_LIBRARY_H

#include <Arduino.h>
#include <SD.h>

#include "ButtonUtils.h"

class TrackLibrary
{
public:
    static constexpr const char* REGISTRY_FILE = "REG.MMS";

    static constexpr uint8_t MAX_PLAYLIST_TRACKS = 8;
    static constexpr uint8_t COORD_LEN = 2;
    static constexpr uint8_t MAX_FILENAME_LEN = 24;

    static constexpr uint8_t MAX_LINE_LEN =
        MAX_FILENAME_LEN + COORD_LEN + 2;

    struct Playlist
    {
        ButtonLayout::Coord button;
        uint8_t trackCount = 0;
        char tracks[MAX_PLAYLIST_TRACKS][MAX_FILENAME_LEN] = {{0}};
    };

public:
    TrackLibrary();

    bool begin();

    bool loadPlaylist(
        const ButtonLayout::Coord& requestedButton,
        Playlist& playlist
    ) const;

    bool hasTrack(
        const ButtonLayout::Coord& button
    ) const;

private:
    bool parseLine(
        const char* line,
        ButtonLayout::Coord& button,
        char* fileName,
        size_t fileNameSize
    ) const;

    bool coordinatesEqual(
        const ButtonLayout::Coord& first,
        const ButtonLayout::Coord& second
    ) const;

    bool isDigit(char c) const;
    void clearPlaylist(Playlist& playlist) const;

private:
    bool _initialized;
};

#endif