call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86
call configure.bat msvc9
call cd build\lib
call nmake clean install