#!/bin/sh
CPU_CORES=`grep -c ^processor /proc/cpuinfo`
USED_CPU_CORES=$(($CPU_CORES + 1))
if [ -z "$CMAKE_BIN" ]; then CMAKE_BIN='cmake'; fi

`$CMAKE_BIN ..`

echo "#### Using $USED_CPU_CORES parallel compile jobs ###"

make -j $USED_CPU_CORES
