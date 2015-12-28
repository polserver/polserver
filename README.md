[![Build Status](https://travis-ci.org/polserver/polserver.svg?branch=master)](https://travis-ci.org/polserver/polserver)
[![Build status](https://ci.appveyor.com/api/projects/status/trv2pbr1icve49sp?svg=true)](https://ci.appveyor.com/project/turleypol/polserver)
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

Make sure you have the required libraries installed: boost, zlib and the mysql client (if SQL supported is desired), then run the following commands:

* `cd pol-core/bin-build`
* `./build_linux.sh`

Minimum compiler version: GCC 4.7 or LLVM/Clang 3.5.

Additionally, you need at least CMake 2.8.
