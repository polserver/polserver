call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86_amd64
call configure.bat msvc10x64
call cd build\lib
call nmake clean install