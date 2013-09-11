set ProgRoot=%ProgramFiles%
if not "%ProgramFiles(x86)%" == "" set ProgRoot=%ProgramFiles(x86)%
call "%ProgRoot%\Microsoft Visual Studio 11.0\VC\vcvarsall.bat" x86_amd64

call bootstrap

bjam -j%NUMBER_OF_PROCESSORS% toolset=msvc-11.0 link=static runtime-link=static address-model=64 variant=release --abbreviate-paths --stagedir=lib\x64 --build-dir=build\x64 stage
bjam -j%NUMBER_OF_PROCESSORS% toolset=msvc-11.0 link=static runtime-link=static address-model=64 variant=debug --abbreviate-paths --stagedir=lib\x64 --build-dir=build\x64 stage