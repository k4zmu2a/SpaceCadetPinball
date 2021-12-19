# ARM and Windows XP build tools are optional VS components
msbuild /t:Build /p:Configuration=Release /p:Platform=x86 /v:m SpaceCadetPinball.sln  
msbuild /t:Build /p:Configuration=Release /p:Platform=x64 /v:m SpaceCadetPinball.sln
msbuild /t:Build /p:Configuration=ReleaseWinXP /p:Platform=x86 /v:m SpaceCadetPinball.sln 
msbuild /t:Build /p:Configuration=ReleaseWinXP /p:Platform=x64 /v:m SpaceCadetPinball.sln 
msbuild /t:Build /p:Configuration=Release /p:Platform=ARM /v:m SpaceCadetPinball.sln 
msbuild /t:Build /p:Configuration=Release /p:Platform=ARM64 /v:m SpaceCadetPinball.sln 


Compress-Archive -Path ".\Win32\Release\SpaceCadetPinball.exe" -DestinationPath ".\Export\ReleaseWC\SpaceCadetPinballx86.zip" -Force
Compress-Archive -Path ".\x64\Release\SpaceCadetPinball.exe" -DestinationPath ".\Export\ReleaseWC\SpaceCadetPinballx64.zip" -Force
Compress-Archive -Path ".\Win32\ReleaseWinXP\SpaceCadetPinball.exe" -DestinationPath ".\Export\ReleaseWC\SpaceCadetPinballx86WinXp.zip" -Force
Compress-Archive -Path ".\x64\ReleaseWinXP\SpaceCadetPinball.exe" -DestinationPath ".\Export\ReleaseWC\SpaceCadetPinballx64WinXp.zip" -Force
Compress-Archive -Path ".\ARM\Release\SpaceCadetPinball.exe" -DestinationPath ".\Export\ReleaseWC\SpaceCadetPinballARM.zip" -Force
Compress-Archive -Path ".\ARM64\Release\SpaceCadetPinball.exe" -DestinationPath ".\Export\ReleaseWC\SpaceCadetPinballARM64.zip" -Force