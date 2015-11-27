#!/bin/sh
CPU_CORES=`grep -c ^processor /proc/cpuinfo`
SYSTEM=`lsb_release -d -s`
PHYS_MEM=$((`grep MemTotal /proc/meminfo | awk '{print $2}'`))
PHYS_MEM_GB=`echo "scale=1; ($PHYS_MEM / 1024 / 1024)" | bc -l`
CMAKE_VERSION=`cmake --version | grep 'cmake version' | awk '{print $3}'`

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
cmake ../..

echo "################################################################################"
echo "########################### Compiling + Linking  ###############################"
echo "################################################################################"
USED_CPU_CORES=$(($CPU_CORES + 1))
if [ -n "$TRAVIS_CI" ]; then USED_CPU_CORES=2; fi
echo "############################ Using $USED_CPU_CORES CPU cores  ###############################"
make -j $USED_CPU_CORES
