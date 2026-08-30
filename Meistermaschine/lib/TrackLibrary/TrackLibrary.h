#ifndef TRACK_LIBRARY_H
#define TRACK_LIBRARY_H

#include <Arduino.h>
#include <SD.h>

#include "ButtonUtils.h"

class TrackLibrary
{
public:
    static constexpr uint8_t MAX_PLAYLIST_TRACKS = 8;
    static constexpr uint8_t COORD_LEN = 2;

    // Example: "/Medieval/battle.mp3"
    static constexpr uint8_t MAX_PATH_CHARS = 96;
    static constexpr uint8_t MAX_PATH_LEN   = MAX_PATH_CHARS + 1;
    // name shown on display, e.g. "Medieval"
    static constexpr uint8_t MAX_PRESET_NAME_CHARS = 16;
    static constexpr uint8_t MAX_PRESET_NAME_LEN = MAX_PRESET_NAME_CHARS + 1;

    static constexpr uint8_t MAX_FILENAME_LEN = 80;

    static constexpr uint8_t MAX_LINE_LEN = MAX_FILENAME_LEN + COORD_LEN + 2;

    struct Playlist
    {
        ButtonLayout::Coord button;
        uint8_t trackCount = 0;

        // Entries now contain complete SD paths.
        char tracks[MAX_PLAYLIST_TRACKS][MAX_PATH_LEN] = {{0}};
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

    const char* presetName() const;

private:
    bool hasMmsExtension(const char* fileName) const;

    bool parseLine(
        const char* line,
        ButtonLayout::Coord& button,
        char* fileName,
        size_t fileNameSize
    ) const;

    bool buildTrackPath(
        const char* fileName,
        char* trackPath,
        size_t trackPathSize
    ) const;

    bool coordinatesEqual(
        const ButtonLayout::Coord& first,
        const ButtonLayout::Coord& second
    ) const;

    bool isDigit(char c) const;
    void clearPlaylist(Playlist& playlist) const;

    bool findPreset();

private:
    bool _initialized;

    char _presetName[MAX_PRESET_NAME_LEN];
    char _presetPath[MAX_PATH_LEN];
    char _registryPath[MAX_PATH_LEN];
};

#endif