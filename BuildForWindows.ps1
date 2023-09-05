#Run this script from Developer Command Prompt for VS *
$artefacts = ".\bin\Release\SpaceCadetPinball.exe", ".\bin\Release\SDL2.dll", ".\bin\Release\SDL2_mixer.dll"

#X86 build
Remove-Item -Path .\build\CMakeCache.txt -ErrorAction SilentlyContinue
cmake -S . -B build -A Win32 -DCMAKE_WIN32_EXECUTABLE:BOOL=1
cmake --build build --config Release
Compress-Archive -Path $artefacts -DestinationPath ".\bin\SpaceCadetPinballx86Win.zip" -Force

#X64 build
Remove-Item -Path .\build\CMakeCache.txt
cmake -S . -B build -A x64 -DCMAKE_WIN32_EXECUTABLE:BOOL=1
cmake --build build --config Release
Compress-Archive -Path $artefacts -DestinationPath ".\bin\SpaceCadetPinballx64Win.zip" -Force

#86 XP build, requires special XP MSVC toolset
Remove-Item -Path .\build\CMakeCache.txt
cmake -S . -B build -A Win32 -DCMAKE_WIN32_EXECUTABLE:BOOL=1 -T v141_xp
cmake --build build --config Release
Compress-Archive -Path $artefacts -DestinationPath ".\bin\SpaceCadetPinballx86WinXP.zip" -Force