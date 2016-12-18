#!/bin/sh

prune () {
	make clean
	rm -r -f pol-core
	rm -r -f lib
	rm -r -f CMakeFiles
	rm CMakeCache.txt
	rm Makefile
}

case "$1" in
	-g|-gcc5)
		export CXX="g++-5" CC="gcc-5";
		echo "FORCING GCC-5"
		prune
		./build_linux.sh
		;;
	-c|-clang37)
		export CXX="clang++-3.7" CC="clang-3.7";
		echo "FORCING CLANG-3.7"
		prune
		./build_linux.sh
		;;
	-p|-prune)
		echo "PRUNE.."
		prune
		exit
		;;
	-t|-tidy)
		export CXX="clang++-3.7" CC="clang-3.7";
		echo "CLANG TIDY check.."
		prune
		CMAKE_BIN='cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON'
		`$CMAKE_BIN ../..`
		/usr/lib/llvm-3.7/share/clang/run-clang-tidy.py -clang-tidy-binary=clang-tidy-3.7 2>&1 | tee tidy.log
		exit
		;;
	-b|-benchmark)
		echo "BENCHMARK build.."
		CMAKE_BIN='cmake -DENABLE_BENCHMARK=1'
		`$CMAKE_BIN ../..`
		exit
		;;
	*)
		;;
esac
