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
goto begin


:begin
set STARTTIME=%TIME%

echo Extracting external libraries (boost, curl)
pushd ..\lib
call prepare_extlibs.bat
if %errorlevel% neq 0 goto :error
popd

call %VCVARSALL% x64
if %errorlevel% neq 0 goto :error

pushd ..\lib\curl-7.57.0\winbuild

rem Used in libcurl to statically link CRT
set RTLIBCFG=static
nmake /f Makefile-libcurl-polserver.vc mode=static machine=x64 VC=%VC% ENABLE_WINSSL=yes DEBUG=no
if %errorlevel% neq 0 goto :error
nmake /f Makefile-libcurl-polserver.vc mode=static machine=x64 VC=%VC% ENABLE_WINSSL=yes DEBUG=yes
if %errorlevel% neq 0 goto :error
popd


echo Building x64-release from %POLSOL%...
msbuild %POLSOL% /m /p:Configuration=Release /p:Platform="x64" > dist\buildlog_64.log
if %errorlevel% neq 0 goto :error

echo Packing everything up...
pushd dist
call mkdist x64 clean
call mkdist x64 dist 
call mkdist x64 obj
call mkdist x64 lib
call mkdist x64 src
if %errorlevel% neq 0 goto :error
popd

call %VCVARSALL% x86
if %errorlevel% neq 0 goto :error

pushd ..\lib\curl-7.57.0\winbuild
nmake /f Makefile-libcurl-polserver.vc mode=static machine=x86 VC=%VC% ENABLE_WINSSL=yes DEBUG=no
if %errorlevel% neq 0 goto :error
nmake /f Makefile-libcurl-polserver.vc mode=static machine=x86 VC=%VC% ENABLE_WINSSL=yes DEBUG=yes
if %errorlevel% neq 0 goto :error
popd

echo Building Win32-release from %POLSOL%...
msbuild %POLSOL% /m /p:Configuration=Release /p:Platform="Win32" > dist\buildlog_32.log
if %errorlevel% neq 0 goto :error
pushd dist
call mkdist x86 clean
call mkdist x86 dist
call mkdist x86 obj
if %errorlevel% neq 0 goto :error
popd

goto end

:vserror
echo Couldnt find a suitable vcvarsall.bat.
pause
goto exit

:error
echo Something went wrong. Check the logs in the 'dist' folder.
pause
goto exit

:end
echo Done. Started at %STARTTIME%, finished at %TIME%.
pause
goto exit


:exit

