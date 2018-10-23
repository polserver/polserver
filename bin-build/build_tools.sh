#!/bin/sh

prune () {
	make clean
	rm -r -f pol-core
	rm -r -f lib
	rm -r -f docs
	rm -r -f CMakeFiles
    rm -r -f Testing
	rm -f CMakeCache.txt
	rm -f Makefile
	rm -f install_manifest.txt
	rm -f CPack*
	rm -f pol_global_config.h
	rm -f pol_revision.h
	rm -f pol_revision.h.tmp
	rm -f cmake_install.cmake
    rm -f build.ninja
    rm -f rules.ninja
    rm -f CTest*
}

case "$1" in
	-g|-gcc5)
		export CXX="g++-5" CC="gcc-5";
		echo "FORCING GCC-5"
		prune
		./build_linux.sh
		;;
	-c|-clang37)
		export CXX="clang++" CC="clang";
		echo "FORCING CLANG"
		prune
		./build_linux.sh
		;;
	-p|-prune)
		if [ ${PWD##*/} = "bin-build" ]; then
		  echo "PRUNE.."
		  prune
		fi
		exit
		;;
	-t|-tidy)
		export CXX="clang++-3.7" CC="clang-3.7";
		echo "CLANG TIDY check.."
		prune
		CMAKE_BIN='cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON'
		`$CMAKE_BIN ..`
		/usr/lib/llvm-3.7/share/clang/run-clang-tidy.py -clang-tidy-binary=clang-tidy-3.7 2>&1 | tee tidy.log
		exit
		;;
	-b|-benchmark)
		echo "BENCHMARK build.."
		CMAKE_BIN='cmake -DENABLE_BENCHMARK=1'
		`$CMAKE_BIN ..`
		exit
		;;
	*)
		;;
esac
