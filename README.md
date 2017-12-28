[![Build Status](https://travis-ci.org/polserver/polserver.svg?branch=master)](https://travis-ci.org/polserver/polserver)
[![Build status](https://ci.appveyor.com/api/projects/status/trv2pbr1icve49sp/branch/master?svg=true)](https://ci.appveyor.com/project/turleypol/polserver/branch/master)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/2862/badge.svg)](https://scan.coverity.com/projects/2862)


POL
=========

PenUltima Online (POL) is a MMO server, compatible with most Ultima Online clients.  POL is customizable via configuration files and has its own scripting language (eScript).

More information: http://www.polserver.com. <br/>
Forum: http://forums.polserver.com/ <br/>
Docs: http://docs.polserver.com/

IRC: #pol @ irc.darkmyst.org

If you find bugs, please create an issue with the dump, relevant log lines and your best guess on how to reproduce the bug. 

<b>Current version: POL 099 (beta).</b>

Contributing
---------

You can send us pull requests at anytime. We expect you to be the author of the contributed code or that it's in a compatible license.

We always need updates or better descriptions in the docs, so feel free to send pull requests for changing them. You can find some of them in `docs/docs.polserver.com/pol099`.

The best way to get used to the code is to search for eScript functions in the cpp files. For example, "SystemFindObjectBySerial" or "ApplyDamage". This way you can see how eScript interfaces with the core and how to add/modify modules and functions.


Compiling in Windows
---------

If you're compiling it for the first time, you need to unpack the boost library by running `prepare_boost.bat` in the `lib/` folder. 

In the folder `pol-core`, run `buildDistWin.bat` to create a complete package in `dist`. Alternatively, open the Visual Studio solution (e.g., pol-2012.sln) and choose to build the solution (Ctrl+Shift+B).
 
Minimum compiler version: Visual Studio 2012 (Express editions might require the Windows SDK to be installed.)

<b>Warning:</b> We can not easily debug dumps from self-compiled cores. Make sure you save the `.pdb` and `.exe`!
 
Compiling in Linux
----------

Make sure you have the required libraries installed: boost, zlib, tinyxml and the mysql client (if SQL supported is desired).
Boost libraries are also shipped with the source. If you choose to use the provided version, unpack them with:
* `cd lib`
* `tar xjf boost_1_63_0.tar.bz2`
* `cd ..`

Then run the following commands (you need at least CMake 2.8):
* `cd pol-core/bin-build`
* `./build_linux.sh`

Minimum compiler version: GCC 4.9 or LLVM/Clang 3.5.
For compiling with GCC 4.7 and 4.8, please see the `Advanced Compiling` section on how to enable `USE_BOOST_REGEX=1`

The command will generate binaries and dynamicaly linked libraries in the `bin/` folder. You need them all for your binaries to work. Debug symbols are included by default.

To generate a compressed distributable archive (without debug symbols), then use `./build_linux_release.sh`.

### Manual/Advanced compiling

The following instructions are only for advanced users. If you only want to compile the core, please follow the steps above.

This software uses a CMake-based build system. CMake is a build system generator: on Linux, it will take care of generating a Makefile.
If you are familiar with linux source tarballs, just think of *cmake* as a modern replacement for the good old *./configure* script.

First of all, run cmake one first time and let it do some autodetection:
```
cd pol-core
cmake -Wno-dev
```

A file called `CMakeCache.txt` has now been created: inside this file, you will find all the settings that will be used for your build. You have many options to edit this file:
* by hand, with your favorite text editor (read the comments in the file header for a quick syntax briefing)
* from a terminal GUI, like `ccmake` (`sudo apt-get install cmake-curses-gui` to install it on Debian-based systems): just launch the gui by typing `ccmake .` and follow the interface
* from a X window GUI, like `cmake-gui` (`sudo apt-get install cmake-qt-gui` to install it on Debian-based systems): just launch the gui by typing `cmake-gui .` and follow the interface

Here is a non-complete description of the most useful options:
* `FORCE_ARCH_BITS='<bits>'` replace \<bits\> with 32 to force a 32 bit build, 64, to force a 64 bit build. The default settings use autodetection.
* `BUILD='<type>'` replace \<type\> with one of the following:
  * `Default`: runs the default build using default settings
  * `Debug`: makes a debug build (adds '-O2', '-g', '--ggdb')
  * `Valgrind`: makes a special debug build to be used in Valgrind that will run very slowly (like *Debug*, but uses '-O0' instead of '-O2')
* `USE_BOOST_REGEX=<1|0>` if set to 1, uses boost instead of std regex to work around a GCC bug in versions prior to 4.9

You can also set an option directly with the '-D' cmake flag, by example:
```
cmake -Wno-dev -DFORCE_ARCH_BITS=32 -DBUILD=Debug
```
Keep in mind that even when set this way, the option will be saved in the `CMakeCache.txt` configuraton file and remembered for future builds.

You can now use the classic *make* command to compile the POL. You can type:
* `make` or `make all` to do the build
* `make clean` to delete all the intermediate build files

You will find the compiled files inside the `bin/` folder.
