[![Build Status](https://github.com/polserver/polserver/workflows/Build/badge.svg)](https://github.com/polserver/polserver/actions)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/2862/badge.svg)](https://scan.coverity.com/projects/2862)
[![Coverage Status](https://coveralls.io/repos/github/polserver/polserver/badge.svg?branch=master)](https://coveralls.io/github/polserver/polserver?branch=master)

POL
=========

PenUltima Online (POL) is a MMO server, compatible with most Ultima Online clients.  POL is customizable via configuration files and has its own scripting language (eScript). POL has been tested and run on both Windows and Linux natively, and builds are offered for both.

More information: http://www.polserver.com. <br/>
Forum: http://forums.polserver.com/ <br/>
Docs: http://docs.polserver.com/

Discord: https://discord.gg/4JbC8hA

If you find bugs, please create an issue with the dump, relevant log lines and your best guess on how to reproduce the bug. 

<b>Current stable version: POL 100.0.0</b>

Always check the core-changes.txt file for breaking changes when using the development version. We'll strive to keep the development version stable. If you are looking for POL099, check the branch "POL099". From now on, POL099 will only receive bug-fixes.


Contributing
---------

You can send us pull requests at anytime. We expect you to be the author of the contributed code or that it's in a compatible license.

We always need updates or better descriptions in the docs, so feel free to send pull requests for changing them. You can find some of them in `docs/docs.polserver.com/pol100`.

The best way to get used to the code is to search for eScript functions in the cpp files. For example, "SystemFindObjectBySerial" or "ApplyDamage". This way you can see how eScript interfaces with the core and how to add/modify modules and functions.


Compiling in Windows
---------

You need at least cmake version 3.9.

**Recommended:**
Open the *CMake GUI* select the the root folder as source code folder, select
the `bin-build` subfolder as "where to build the binaries" folder, and select
your installed Visual Studio version e.g. `Visual Studio 14 2015 Win64`.
Now simply press configure+generate+open project and it will directly open
the solution file in your IDE.
After building the binaries are located in the `bin` folder.

**Not Recommended:**
If you are using *Visual Studio 2017*, you can try using the integraded CMake:
use the `open folder` function and select the root folder. CMake will take a
while to load: wait until the `CMAKE` manu appears, then you can use it to
start the build, make sure to have the `Realease` mode selected.

Minimum compiler version: Visual Studio 2017

<b>Warning:</b> We can not easily debug dumps from self-compiled cores. Make sure you save the `.pdb` and `.exe`!
 
Compiling in Linux
----------

Make sure you have the required libraries installed: zlib, openssl and the mysql client (if SQL supported is desired).

Then run the following commands (you need at least CMake 3.9):
* `cd bin-build`
* `./build_linux.sh`

Minimum compiler version: GCC 8 or LLVM/Clang 7.

The command will generate binaries and dynamicaly linked libraries in the `bin/` folder. You need them all for your binaries to work. Debug symbols are included by default.

To generate a compressed distributable archive (without debug symbols), then use `./build_linux_release.sh`.

### Manual/Advanced compiling

The following instructions are only for advanced users. If you only want to compile the core, please follow the steps above.

This software uses a CMake-based build system. CMake is a build system generator: on Linux, it will take care of generating a Makefile.
If you are familiar with linux source tarballs, just think of *cmake* as a modern replacement for the good old *./configure* script.

First of all, run cmake one first time and let it do some autodetection:
```
cd bin-build
cmake ..
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

You can also set an option directly with the '-D' cmake flag, by example:
```
cmake -Wno-dev -DFORCE_ARCH_BITS=32 -DBUILD=Debug
```
Keep in mind that even when set this way, the option will be saved in the `CMakeCache.txt` configuraton file and remembered for future builds.

You can now use the classic *make* command to compile the POL. You can type:
* `make` or `make all` to do the build

You will find the compiled files inside the `bin/` folder.

To delete intermediate build files and clean the folder you can use the
interective clean.py script.
