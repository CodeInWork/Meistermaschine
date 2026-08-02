# MEISTERMASCHINE

**MEISTERMASCHINE** is a dedicated hardware audio controller designed as the physical companion to the **MEISTERMASCHINE Desktop App**. Together, they provide an intuitive solution for creating, organizing, and playing ambient soundscapes and music during tabletop role-playing games.

The project is primarily intended for **Dungeon Masters (DMs)** who want to enhance the atmosphere of their games without having to manage playlists or operate a computer during play. Instead of navigating through folders or media players, the DM can trigger carefully prepared sounds and music with a single button press, allowing them to remain focused on storytelling and game management.

## Features

* Dedicated hardware interface for instant audio playback
* Supports customizable sound presets created with the MEISTERMASCHINE Desktop App
* Simple button-based operation designed for use during live tabletop sessions
* LCD display showing the currently loaded preset and active audio track
* SD card based storage for presets and audio files
* Compact, standalone design requiring no computer during gameplay

## How It Works

Audio presets are created with the **MEISTERMASCHINE Desktop App** and exported to an SD card.

Each preset consists of:

* a dedicated preset folder
* a registry (`*.mms`) describing the button-to-audio mapping
* the associated audio files

The hardware automatically detects the first valid preset on the SD card and loads it during startup. Once loaded, sounds and music can be triggered directly using the physical button matrix.

## Design Goals

The project was developed with the following principles in mind:

* **Immersion first** – Enhance the atmosphere without interrupting gameplay.
* **Minimal distraction** – Allow the Dungeon Master to focus on narration instead of operating software.
* **Reliability** – Dedicated hardware provides predictable behavior during long gaming sessions.
* **Simplicity** – A clear interface with immediate access to frequently used sounds.

## Hardware

The current hardware is based on:

* Arduino Nano Every
* VS1053 MP3 decoder
* MCP23017 I²C port expanders
* SD card storage
* Character LCD display
* Stereo audio amplifier
* 20-button matrix interface

## Software

This repository contains the firmware running on the physical controller.

The companion **MEISTERMASCHINE Desktop App** is responsible for:

* creating and editing presets
* assigning audio files to buttons
* generating `.mms` registry files
* exporting complete presets to an SD card

## Project Status

MEISTERMASCHINE is an active hobby project and is continuously evolving. Hardware revisions and firmware improvements are made whenever new features or usability enhancements are identified.

Contributions, suggestions, and feedback are always welcome.
