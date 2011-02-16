set ProgRoot=%ProgramFiles%
if not "%ProgramFiles(x86)%" == "" set ProgRoot=%ProgramFiles(x86)%
call "%ProgRoot%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86_amd64
call configure.bat msvc10x64
call cd build\lib
call nmake clean install