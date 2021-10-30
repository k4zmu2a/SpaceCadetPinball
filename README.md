# SpaceCadetPinball
**Summary:** Reverse engineering of `3D Pinball for Windows – Space Cadet`, a game bundled with Windows.

**How to play:** Place compiled exe into a folder containing original game resources (not included).\
Supports data files from Windows and Full Tilt versions of the game.
\
\
\
\
\
\
**Source:**
 * `pinball.exe` from `Windows XP` (SHA-1 `2A5B525E0F631BB6107639E2A69DF15986FB0D05`) and its public PDB
 * `CADET.EXE` 32bit version from `Full Tilt! Pinball` (SHA-1 `3F7B5699074B83FD713657CD94671F2156DBEDC4`)

**Tools used:** `Ghidra`, `Ida`, `Visual Studio`

**What was done:**
 * All structures were populated, globals and locals named.
 * All subs were decompiled, C pseudo code was converted to compilable C++. Loose (namespace?) subs were assigned to classes.

**Compiling:**\
Project uses `C++11` features and depends on Windows libs.\
Compile with Visual Studio; tested with 2017 and 2019. 

**Plans:**
 * ~~Decompile original game~~
 * ~~Resizable window, scaled graphics~~
 * ~~Loader for high-res sprites from CADET.DAT~~
 * ~~Cross-platform port using SDL2, SDL2_mixer, ImGui~~
 * Text translations
 * Misc features of Full Tilt: 3 music tracks, multiball, centered textboxes, etc.
 * Maybe x2: support for other two tables 
   * Table specific BL (control interactions and missions) is hardcoded, othere parts might be also patched

**On 64-bit bug that killed the game:**\
I did not find it, decompiled game worked in x64 mode on the first try.\
It was either lost in decompilation or introduced in x64 port/not present in x86 build.\
Based on public description of the bug (no ball collision), I guess that the bug was in `TEdgeManager::TestGridBox`
