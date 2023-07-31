#!/usr/bin/env bash

set -xe

mkdir -p Libs

cd Libs

sdl_version='2.28.1'
sdl_filename="SDL2-$sdl_version.dmg"
sdl_url="https://github.com/libsdl-org/SDL/releases/download/release-$sdl_version/$sdl_filename"

sdl_mixer_version='2.7.0'
sdl_mixer_filename="SDL2_mixer-$sdl_mixer_version.dmg"
sdl_mixer_url="https://www.libsdl.org/tmp/$sdl_mixer_filename"

mount_point="$(mktemp -d)"

if [ ! -f "$sdl_filename" ]; then
	curl -sSf -L -O "$sdl_url"
fi

echo "2f936225c10a402cab07055e6f75de76f991945c37feb0cf6af633a96d2fb28c  $sdl_filename" | shasum -a 256 -c
hdiutil attach "$sdl_filename" -mountpoint "$mount_point" -quiet
cp -a "$mount_point/SDL2.framework" .
hdiutil detach "$mount_point"

if [ ! -f "$sdl_mixer_filename" ]; then
	curl -sSf -L -O "$sdl_mixer_url"
fi

echo "f394c714c8aefdcae0ff9d6eefeb5d42f28e56ed09fcaebb796cb672ca11279d  $sdl_mixer_filename" | shasum -a 256 -c
hdiutil attach "$sdl_mixer_filename" -mountpoint "$mount_point" -quiet
cp -a "$mount_point/SDL2_mixer.framework" .
hdiutil detach "$mount_point"

cd ..

cmake .
cmake --build .

sw_version='2.0.1'

mkdir -p SpaceCadetPinball.app/Contents/MacOS
mkdir -p SpaceCadetPinball.app/Contents/Resources
mkdir -p SpaceCadetPinball.app/Contents/Frameworks

cp -a Platform/macOS/Info.plist SpaceCadetPinball.app/Contents/
cp -a Platform/macOS/SpaceCadetPinball.icns SpaceCadetPinball.app/Contents/Resources/
cp -a Libs/SDL2.framework SpaceCadetPinball.app/Contents/Frameworks/
cp -a Libs/SDL2_mixer.framework SpaceCadetPinball.app/Contents/Frameworks/
cp -a bin/SpaceCadetPinball SpaceCadetPinball.app/Contents/MacOS/

sed -i '' "s/CHANGEME_SW_VERSION/$sw_version/" SpaceCadetPinball.app/Contents/Info.plist

echo -n "APPL????" > SpaceCadetPinball.app/Contents/PkgInfo

hdiutil create -fs HFS+ -srcfolder SpaceCadetPinball.app -volname "SpaceCadetPinball $sw_version" "SpaceCadetPinball-$sw_version-mac.dmg"

rm -r SpaceCadetPinball.app
