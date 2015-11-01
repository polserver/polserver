#!/bin/sh

./build_linux.sh
SOURCE_DIR_BUILD_BIN=../bin
DEST_DIR_DISTRI="RELEASE"
POL_RELEASE="099"
SYSTEM=`lsb_release -i -s`
DEST_TAR_FILE="../pol-LINUX-$POL_RELEASE-`date +%Y-%m-%d`-$SYSTEM.tar"

###########################################################
###########################################################

echo "############## Creating directories"
mkdir -p $DEST_DIR_DISTRI
mkdir -p $DEST_DIR_DISTRI/scripts/modules
mkdir -p $DEST_DIR_DISTRI/config

echo "############## Removing debug symbols and copying binaries to RELEASE folder"
strip -o $DEST_DIR_DISTRI/pol              $SOURCE_DIR_BUILD_BIN/pol
strip -o $DEST_DIR_DISTRI/scripts/ecompile $SOURCE_DIR_BUILD_BIN/ecompile
strip -o $DEST_DIR_DISTRI/scripts/runecl   $SOURCE_DIR_BUILD_BIN/runecl
strip -o $DEST_DIR_DISTRI/uoconvert        $SOURCE_DIR_BUILD_BIN/uoconvert
strip -o $DEST_DIR_DISTRI/uotool           $SOURCE_DIR_BUILD_BIN/uotool

echo "############## Copying additionally needed files to RELEASE folder"
cp ../support/scripts/*.em                 $DEST_DIR_DISTRI/scripts/modules
cp ../support/scripts/ecompile.cfg.example $DEST_DIR_DISTRI/scripts
cp ../doc/core-changes.txt                 $DEST_DIR_DISTRI/
cp ../doc/uoconvert.txt                    $DEST_DIR_DISTRI/
cp ../doc/packethooks.txt                  $DEST_DIR_DISTRI/
cp ../doc/packets.zip                      $DEST_DIR_DISTRI/
cp ../support/uoconvert.cfg                $DEST_DIR_DISTRI/
cp ../support/boats.cfg                    $DEST_DIR_DISTRI/config
cp ../support/extobj.cfg                   $DEST_DIR_DISTRI/config
cp ../support/animxlate.cfg                $DEST_DIR_DISTRI/config

cd $DEST_DIR_DISTRI/
echo "############## Creating release archive"
tar -cvf $DEST_TAR_FILE pol uoconvert uotool core-changes.txt uoconvert.txt uoconvert.cfg config/boats.cfg config/extobj.cfg config/animxlate.cfg packethooks.txt packets.zip scripts
echo "############## Compressing release archive"
gzip -f -9 $DEST_TAR_FILE
echo "############## Removing temporary files"
cd ..
rm -rf $DEST_DIR_DISTRI