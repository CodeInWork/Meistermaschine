#ifndef TRACK_LIBRARY_H
#define TRACK_LIBRARY_H

#include <Arduino.h>
#include <SD.h>

#include "ButtonUtils.h"

class TrackLibrary
{
public:
    static constexpr uint8_t MAX_PLAYLIST_TRACKS = 12;
    static constexpr uint8_t COORD_LEN = 2;

    // Preset name shown on display
    static constexpr uint8_t MAX_PRESET_NAME_CHARS = 16;
    static constexpr uint8_t MAX_PRESET_NAME_LEN = MAX_PRESET_NAME_CHARS + 1;

    // Machine filename: "T000001.MP3"
    static constexpr uint8_t MAX_FILENAME_CHARS = 11;
    static constexpr uint8_t MAX_FILENAME_LEN = MAX_FILENAME_CHARS + 1;

    // "/PresetName/T000001.MP3"
    static constexpr uint8_t MAX_TRACK_PATH_CHARS = 1 + MAX_PRESET_NAME_CHARS + 1 + MAX_FILENAME_CHARS;

    static constexpr uint8_t MAX_TRACK_PATH_LEN = MAX_TRACK_PATH_CHARS + 1;

    // "PresetName.mms"
    static constexpr uint8_t MAX_REGISTRY_NAME_CHARS = MAX_PRESET_NAME_CHARS + 4;

    // "/PresetName/PresetName.mms"
    static constexpr uint8_t MAX_REGISTRY_PATH_CHARS = 1 + MAX_PRESET_NAME_CHARS + 1 + MAX_REGISTRY_NAME_CHARS;

    static constexpr uint8_t MAX_REGISTRY_PATH_LEN = MAX_REGISTRY_PATH_CHARS + 1;

    static constexpr uint8_t MAX_TRACK_TITLE_CHARS = 32;
    static constexpr uint8_t MAX_TRACK_TITLE_LEN = MAX_TRACK_TITLE_CHARS + 1;

    static constexpr uint8_t MAX_PRESET_PATH_CHARS = 1 + MAX_PRESET_NAME_CHARS;

    static constexpr uint8_t MAX_PRESET_PATH_LEN = MAX_PRESET_PATH_CHARS + 1;

    static constexpr uint8_t MAX_LINE_CHARS = COORD_LEN + 1 + MAX_FILENAME_CHARS + 1 + MAX_TRACK_TITLE_CHARS;

    static constexpr uint8_t MAX_LINE_LEN = MAX_LINE_CHARS + 1;

    struct Playlist
    {
        ButtonLayout::Coord button;
        uint8_t trackCount = 0;

        char tracks[MAX_PLAYLIST_TRACKS][MAX_TRACK_PATH_LEN] = {{0}};
        char titles[MAX_PLAYLIST_TRACKS][MAX_TRACK_TITLE_LEN] = {{0}};
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
        size_t fileNameSize,
        char* title,
        size_t titleSize
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
    char _presetPath[MAX_PRESET_PATH_LEN];  // "/" + name + '\0'
    char _registryPath[MAX_REGISTRY_PATH_LEN];
};

#endif