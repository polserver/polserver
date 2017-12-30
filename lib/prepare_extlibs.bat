@echo off

rem Attempts to find the most recent visual studio.

:VS2015
set VCVARSALL="%VS140COMNTOOLS%..\..\VC\vcvarsall.bat"
set POLSOL="pol-2015.sln"
set VC=15
if not exist %VCVARSALL% goto VS2013
goto begin

:VS2013
set VCVARSALL="%VS120COMNTOOLS%..\..\VC\vcvarsall.bat"
set POLSOL="pol-2013.sln"
set VC=13
if not exist %VCVARSALL% goto VS2012
goto begin

:VS2012
set VCVARSALL="%VS110COMNTOOLS%..\..\VC\vcvarsall.bat"
set POLSOL="pol-2012.sln"
set VC=12
if not exist %VCVARSALL% goto vserror

:begin

if exist boost_1_63_0\boost goto extract_curl
..\pol-core\dist\7za.exe x -so boost_1_63_0.tar.bz2 | ..\pol-core\dist\7za.exe x -si -ttar

:extract_curl
if exist curl-7.57.0 goto done
..\pol-core\dist\7za.exe x curl-7.57.0.zip

call %VCVARSALL% x64
if %errorlevel% neq 0 goto :error

pushd ..\lib\curl-7.57.0\winbuild

rem Used in libcurl to statically link CRT
set RTLIBCFG=static
nmake /f ..\..\Makefile-libcurl-polserver.vc mode=static machine=x64 VC=%VC% ENABLE_WINSSL=yes DEBUG=no
if %errorlevel% neq 0 goto :error
nmake /f ..\..\Makefile-libcurl-polserver.vc mode=static machine=x64 VC=%VC% ENABLE_WINSSL=yes DEBUG=yes
if %errorlevel% neq 0 goto :error
popd

:done
echo Finished extracting files.
goto exit 

:vserror
echo Couldnt find a suitable vcvarsall.bat.
pause
goto exit

:error
echo Something went wrong. Check the logs in the 'dist' folder.
pause

goto exit

:exit 

