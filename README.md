<!-- markdownlint-disable-file MD033 -->

# SpaceCadetPinball

## Summary

Reverse engineering of `3D Pinball for Windows - Space Cadet`, a game bundled with Windows.

3D Pinball for Windows - Space Cadet was a pinball video game developed by Cinematronics and published by Maxis in 1995.

## How to play

Place compiled executable into a folder containing original game resources (not included).\
Supports data files from Windows and Full Tilt versions of the game.

## Known source ports

| Platform        | Author          | URL                                                                                                        |
| --------------- | --------------- | ---------------------------------------------------------------------------------------------------------- |
| PS Vita         | Axiom           | <https://github.com/suicvne/SpaceCadetPinball_Vita>                                                        |
| Emscripten      | alula           | <https://github.com/alula/SpaceCadetPinball> <br> Play online: <https://alula.github.io/SpaceCadetPinball> |
| Nintendo Switch | averne          | <https://github.com/averne/SpaceCadetPinball-NX>                                                           |
| webOS TV        | mariotaku       | <https://github.com/webosbrew/SpaceCadetPinball>                                                           |
| Android (WIP)   | Iscle           | https://github.com/Iscle/SpaceCadetPinball                                                                 |
| Nintendo Wii    | MaikelChan      | https://github.com/MaikelChan/SpaceCadetPinball                                                            |
| Nintendo 3DS    | MaikelChan      | https://github.com/MaikelChan/SpaceCadetPinball/tree/3ds                                                   |
| Nintendo Wii U  | IntriguingTiles | https://github.com/IntriguingTiles/SpaceCadetPinball-WiiU                                                  |

Platforms covered by this project: desktop Windows, Linux and macOS.

<br>
<br>
<br>
<br>
<br>
<br>

## Source

* `pinball.exe` from `Windows XP` (SHA-1 `2A5B525E0F631BB6107639E2A69DF15986FB0D05`) and its public PDB
* `CADET.EXE` 32bit version from `Full Tilt! Pinball` (SHA-1 `3F7B5699074B83FD713657CD94671F2156DBEDC4`)

## Tools used

`Ghidra`, `Ida`, `Visual Studio`

## What was done

* All structures were populated, globals and locals named.
* All subs were decompiled, C pseudo code was converted to compilable C++. Loose (namespace?) subs were assigned to classes.

## Compiling

Project uses `C++11` and depends on `SDL2` libs.

### On Windows

Download and unpack devel packages for `SDL2` and `SDL2_mixer`.\
Set paths to them in `CMakeLists.txt`, see suggested placement in `/Libs`.\
Compile with Visual Studio; tested with 2019.

### On Linux

Install devel packages for `SDL2` and `SDL2_mixer`.\

Compile with CMake; tested with GCC 10, Clang 11.\
To cross-compile for Windows, install a 64-bit version of mingw and its `SDL2` and `SDL2_mixer` distributions, then use the `mingwcc.cmake` toolchain.

```bash
# On Debian and Ubuntu
sudo apt install cmake build-essential ninja-build libsdl2-dev libsdl2-mixer-dev libsdl2-mixer-2.0-0 libsdl2-2.0-0 fluidsynth

# On Fedora
sudo dnf install cmake ninja-build SDL2 SDL2-devel SDL2_mixer SDL2_mixer-devel fluidsynth fluidsynth-libs mscore-fonts g++

# Build
cmake -GNinja .
ninja
```

[![Packaging status](https://repology.org/badge/tiny-repos/spacecadetpinball.svg)](https://repology.org/project/spacecadetpinball/versions)

Some distributions provide a package in their repository. You can use those for easier dependency management and updates.

**If you can't hear the background music**: You need to export the following variable before launching the game like this:
`export SDL_SOUNDFONT=/path/to/your/soundfont`

Fluidsynth will install a default soundfont in the following path:
On Fedora: `/usr/share/soundfonts/FluidR3_GM.sf2`
On Ubuntu/Debian: `/usr/share/sounds/sf2/FluidR3_GM.sf2`

### On macOS

* **Homebrew**: Install the `SDL2`, `SDL2_mixer` homebrew packages.
* **MacPorts**: Install the `libSDL2`, `libSDL2_mixer` macports packages.

Compile with CMake. Ensure that `CMAKE_OSX_ARCHITECTURES` variable is set for either `x86_64` Apple Intel or `arm64` for Apple Silicon.

Tested with: macOS Big Sur (Intel) with Xcode 13 & macOS Montery Beta (Apple Silicon) with Xcode 13.

## Plans

* ~~Decompile original game~~
* ~~Resizable window, scaled graphics~~
* ~~Loader for high-res sprites from CADET.DAT~~
* ~~Cross-platform port using SDL2, SDL2_mixer, ImGui~~
* Misc features of Full Tilt: 3 music tracks, multiball, centered textboxes, etc.
* Maybe: Text translations
* Maybe: Android port
* Maybe x2: support for other two tables
  * Table specific BL (control interactions and missions) is hardcoded, othere parts might be also patched

## On 64-bit bug that killed the game

I did not find it, decompiled game worked in x64 mode on the first try.\
It was either lost in decompilation or introduced in x64 port/not present in x86 build.\
Based on public description of the bug (no ball collision), I guess that the bug was in `TEdgeManager::TestGridBox`
