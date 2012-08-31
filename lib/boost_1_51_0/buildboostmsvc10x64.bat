set ProgRoot=%ProgramFiles%
if not "%ProgramFiles(x86)%" == "" set ProgRoot=%ProgramFiles(x86)%
call "%ProgRoot%\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86_amd64

call bootstrap

ECHO using stlport : 5.2 : ../STLport5/stlport : ../STLport5/lib ; >> project-config.jam
ECHO. >> project-config.jam

bjam -j%NUMBER_OF_PROCESSORS% toolset=msvc-10.0 stdlib=stlport link=static runtime-link=static address-model=64 variant=release --abbreviate-paths --stagedir=lib\x64 --build-dir=build\x64 stage
bjam -j%NUMBER_OF_PROCESSORS% toolset=msvc-10.0 stdlib=stlport link=static runtime-link=static address-model=64 variant=debug define=_STLP_DEBUG --abbreviate-paths --stagedir=lib\x64 --build-dir=build\x64 stage