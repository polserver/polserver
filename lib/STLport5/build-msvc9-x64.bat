set ProgRoot=%ProgramFiles%
if not "%ProgramFiles(x86)%" == "" set ProgRoot=%ProgramFiles(x86)%
call "%ProgRoot%\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86_amd64
call configure.bat msvc9x64
call cd build\lib
call nmake clean install