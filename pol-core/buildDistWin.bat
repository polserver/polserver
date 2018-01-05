@echo off

set BUILD_X86=0

set STARTTIME=%TIME%
set POLARCH=x64
echo Extracting external libraries (boost, curl)
pushd ..\lib
call prepare_extlibs.bat
if %errorlevel% neq 0 goto :error
popd

set GITR="Unk Rev"
where git>NUL
if %errorlevel% equ 0 (
  git log -n 1 --pretty="format:%%h" > tmp.log
  set /p GITR=<tmp.log
  del tmp.log
) else (
  echo git not found in PATH!
)
echo Building Revision %GITR%
call :FindReplace "// #define GIT_REVISION" "#define GIT_REVISION `%GITR%`" clib\pol_global_config_win.h

echo Building x64-release from %POLSOL%...
msbuild %POLSOL% /m /p:Configuration=Release /p:Platform="x64" /t:"Clean" > dist\buildlog_64.log
msbuild %POLSOL% /m /p:Configuration=Release /p:Platform="x64" /t:"Build" > dist\buildlog_64.log
if %errorlevel% neq 0 goto :error

echo Packing everything up...
pushd dist
call mkdist x64 clean
call mkdist x64 dist
call mkdist x64 pdb

if %errorlevel% neq 0 goto :error
popd

if %BUILD_X86% neq 1 goto :skipx86
set POLARCH=x86
echo Extracting external libraries (boost, curl)
pushd ..\lib
call prepare_extlibs.bat
if %errorlevel% neq 0 goto :error
popd

echo Building Win32-release from %POLSOL%...
msbuild %POLSOL% /m /p:Configuration=Release /p:Platform="Win32" /t:"Clean" > dist\buildlog_32.log
msbuild %POLSOL% /m /p:Configuration=Release /p:Platform="Win32" /t:"Build" > dist\buildlog_32.log
if %errorlevel% neq 0 goto :error
pushd dist
call mkdist x86 clean
call mkdist x86 dist
call mkdist x86 pdb
if %errorlevel% neq 0 goto :error
popd

:skipx86
rem revert git revision modification
call :FindReplace "#define GIT_REVISION `%GITR%`" "// #define GIT_REVISION" clib\pol_global_config_win.h
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

rem vbs arguments strip "
:FindReplace <findstr> <replstr> <file>
set tmpf="%temp%\tmp.txt"
If not exist %temp%\_pol.vbs call :MakeReplace
for /f "tokens=*" %%a in ('dir "%3" /s /b /a-d /on') do (
    <%%a cscript //nologo %temp%\_pol.vbs "'%~1'" "'%~2'">%tmpf%
    if exist %tmpf% move /Y %tmpf% "%%~dpnxa">nul
)
del %temp%\_pol.vbs
exit /b

:MakeReplace
>%temp%\_pol.vbs echo with Wscript
>>%temp%\_pol.vbs echo set args=.arguments
>>%temp%\_pol.vbs echo .StdOut.Write _
>>%temp%\_pol.vbs echo Replace(.StdIn.ReadAll,replace(replace(args(0),"'",""),"`",""""),replace(replace(args(1),"'",""),"`",""""),1,-1,1)
>>%temp%\_pol.vbs echo end with
exit /b

:exit

