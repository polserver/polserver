call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
call configure.bat msvc10
call cd build\lib
call nmake clean install