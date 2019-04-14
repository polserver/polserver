#!/bin/sh

prune () {
	cmake --build . --target clean
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
	rm -f .ninja_*
}

case "$1" in
	-g|-gcc)
		export CXX="g++" CC="gcc";
		echo "FORCING GCC"
		prune
		cmake -GNinja -DUSE_CCACHE=1 -DNO_PCH=1 ..
		ninja
		;;
	-c|-clang37)
		export CXX="clang++" CC="clang";
		echo "FORCING CLANG"
		prune
		cmake -GNinja -DUSE_CCACHE=1 -DNO_PCH=1 ..
		ninja
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
