#!/bin/sh
CPU_CORES=`grep -c ^processor /proc/cpuinfo`
SYSTEM=`lsb_release -d -s`
PHYS_MEM=$((`grep MemTotal /proc/meminfo | awk '{print $2}'`))
PHYS_MEM_GB=`echo "scale=1; ($PHYS_MEM / 1024 / 1024)" | bc -l`
CMAKE_VERSION=`cmake --version | grep 'cmake version' | awk '{print $3}'`
if [ -z "$CMAKE_BIN" ]; then CMAKE_BIN='cmake'; fi

echo "################################################################################"
echo "############# Starting build of POL from www.polserver.com #####################"
echo "################################################################################"
echo "System           : $SYSTEM"
echo "CPU cores        : $CPU_CORES"
echo "Memory size (GB) : $PHYS_MEM_GB"
echo "CMAKE version    : $CMAKE_VERSION"

echo "################################################################################"
echo "########################### Prepraring CMake  ##################################"
echo "################################################################################"
`$CMAKE_BIN ../..`

echo "################################################################################"
echo "########################### Compiling + Linking  ###############################"
echo "################################################################################"
USED_CPU_CORES=$(($CPU_CORES + 1))
echo "####################### Using $USED_CPU_CORES parallel compile jobs  ########################"
make -j $USED_CPU_CORES
