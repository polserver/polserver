[![Build Status](https://travis-ci.org/polserver/polserver.svg?branch=master)](https://travis-ci.org/polserver/polserver)
[![Coverity Scan Build Status](https://scan.coverity.com/projects/2862/badge.svg)](https://scan.coverity.com/projects/2862)


POL
=========

PenUltima Online (POL) is a MMO server, compatible with most Ultima Online clients.  POL is customizable via configuration files and has its own scripting language (eScript).

More information: http://www.polserver.com. <br/>
Forum: http://forums.polserver.com/ <br/>
Docs: http://docs.polserver.com/

If you find bugs, please create an issue with the dump, relevant log lines and your best guess on how to reproduce the bug. 

<b>Current version: POL 099 (beta).</b>

Compiling in Windows
---------

If you're compiling it for the first time, you need to unpack the boost library by running `prepare_boost.bat` in the `lib/` folder. 

In the folder `pol-core`, run `buildDistWin.bat` to create a complete package in `dist`. Alternatively, open the Visual Studio solution (e.g., pol-2012.sln) and choose to build the solution (Ctrl+Shift+B).
 
Minimum compiler version: Visual Studio 2012 (Express editions might require the Windows SDK to be installed.)

<b>Warning:</b> We can not easily debug dumps from self-compiled cores. Make sure you save the `.pdb` and `.exe`!
 
Compiling in Linux
----------

Make sure you have the required libraries installed: boost, zlib and the mysql client (if SQL supported is desired), then run `./buildcore` in the `pol-core` folder.

Minimum compiler version: GCC 4.8 or LLVM/Clang 3.5.
