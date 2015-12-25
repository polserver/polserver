#!/bin/sh
CPU_CORES=`grep -c ^processor /proc/cpuinfo`
SYSTEM=`lsb_release -d -s`
PHYS_MEM=$((`grep MemTotal /proc/meminfo | awk '{print $2}'`))
PHYS_MEM_GB=`echo "scale=1; ($PHYS_MEM / 1024 / 1024)" | bc -l`
CMAKE_VERSION=`cmake --version | grep 'cmake version' | awk '{print $3}'`
if [ -z "$CMAKE_BIN" ]; then CMAKE_BIN='cmake'; fi

echo "###########################################################################################################"
echo "########################### Starting build of POL from www.polserver.com ##################################"
echo "###########################################################################################################"
echo "System           : $SYSTEM"
echo "CPU cores        : $CPU_CORES"
echo "Memory size (GB) : $PHYS_MEM_GB"
echo "CMAKE version    : $CMAKE_VERSION"

case "$1" in
	-g|-gcc5)
		export CXX="g++-5" CC="gcc-5";
		echo "FORCING GCC-5"
		make clean
		rm -r -f pol-core
		rm -r -f lib
		rm -r -f CMakeFiles
		rm CMakeCache.txt
		rm Makefile
		;;
	-c|-clang37)
		export CXX="clang++-3.7" CC="clang-3.7";
		echo "FORCING CLANG-3.7"
		make clean
		rm -r -f pol-core
		rm -r -f lib
		rm -r -f CMakeFiles
		rm CMakeCache.txt
		rm Makefile
		;;
	-p|-prune)
		make clean
		rm -r -f pol-core
		rm -r -f lib
		rm -r -f CMakeFiles
		rm CMakeCache.txt
		rm Makefile
		exit
		;;
	*)
		;;
esac

echo "###################################################################################"
echo "########################### Prepraring CMake  #####################################"
echo "###################################################################################"
`$CMAKE_BIN ../..`

echo "###################################################################################"
echo "########################### Compiling + Linking  ##################################"
echo "###################################################################################"
USED_CPU_CORES=$(($CPU_CORES + 1))
if [ -n "$TRAVIS_CI" ]; then USED_CPU_CORES=2; fi
echo "############################ Using $USED_CPU_CORES parallel compile jobs  ##################################"
make -j $USED_CPU_CORES
