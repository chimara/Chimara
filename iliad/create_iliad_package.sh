#!/bin/sh

VERSION=0.9

mkdir -p Chimara/chimara
mkdir -p Chimara/interpreters
mkdir -p Chimara/games

# Iliad specific files
cp manifest.xml Chimara/manifest.xml
cp iliad_refresh.conf Chimara/iliad_refresh.conf
cp run.sh Chimara/run.sh
cp style.css Chimara/chimara/style.css
cp chimara.png Chimara/chimara.png

# Chimara lib and player
cp ../libchimara/.libs/libchimara.so.0 Chimara/chimara/
cp ../player/.libs/chimara_iliad Chimara/chimara/chimara

# Interpreters
cp ../interpreters/frotz/.libs/frotz.so Chimara/interpreters/
cp ../interpreters/git/.libs/git.so Chimara/interpreters/
cp ../interpreters/glulxe/.libs/glulxe.so Chimara/interpreters/
cp ../interpreters/nitfol/.libs/nitfol.so Chimara/interpreters/

# Games
cp ../tests/anchor.z8 iliad/Chimara/games/
cp ../tests/CoSv3.blb iliad/Chimara/games/

# Create zip file
zip -r chimara-${VERSION}.zip Chimara

echo "Iliad package created: chimara-${VERSION}.zip"
