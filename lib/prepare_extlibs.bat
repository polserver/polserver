@echo off

rem Attempts to find the most recent visual studio.
if "%POLARCH%" == "" (
set POLARCH=x64
)
set VCWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist %VCWHERE% (
  set VCWHERE="vswhere.exe"
)
for /f "usebackq delims=" %%i in (`%VCWHERE% -prerelease -latest -legacy -property installationPath`) do (
  if exist "%%i\Common7\Tools\vsdevcmd.bat" (
    set VCVARSALL="%%i\Common7\Tools\vsdevcmd.bat"
	set ARCHPARAM=-arch=%POLARCH%
  )
    if exist "%%i\VC\vcvarsall.bat" (
	  set VCVARSALL="%%i\VC\vcvarsall.bat"
	  set ARCHPARAM=%POLARCH%
	)
  
)
if not exist %VCVARSALL% ( 
  goto :vserror
)
for /f "usebackq delims=." %%j in (`%VCWHERE% -prerelease -latest -legacy -property installationVersion`) do (
    set VC=%%j
	set POLSOL=pol-2015.sln
	if "%%j"=="14" (
	  set POLSOL=pol-2015.sln
	)
	if "%%j"=="13" (
	  set POLSOL=pol-2013.sln
	)
	goto :begin
)

:begin

call %VCVARSALL% %ARCHPARAM%
if %errorlevel% neq 0 goto :error

if exist boost_1_63_0\boost goto extract_curl
..\pol-core\dist\7za.exe x -so boost_1_63_0.tar.bz2 | ..\pol-core\dist\7za.exe x -si -ttar

:extract_curl
if exist curl-7.57.0 goto buildcurl
..\pol-core\dist\7za.exe x curl-7.57.0.zip

:buildcurl
if exist curl-7.57.0\builds\libcurl-%POLARCH%-release-static goto done
pushd ..\lib\curl-7.57.0\winbuild

rem Used in libcurl to statically link CRT
set RTLIBCFG=static
nmake /f ..\..\Makefile-libcurl-polserver.vc mode=static machine=%POLARCH% VC=%VC% ENABLE_WINSSL=yes DEBUG=no
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

