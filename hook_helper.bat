@echo off

rem
rem  This scripts creates a symbolic link to git_hooks (under version control) from
rem  the folder ".git/hooks"
rem
rem  It requires admin privileges because of "mklink". If you don't have them, copy
rem  all files manually.
rem
rem

if exist ".git/hooks" (goto needToDelete) else goto install


:needToDelete
choice /M "Folder already exists. Continue?"
if errorlevel 2 goto fail
rmdir /S ".git\hooks"

:install
echo Creating the symbolic link...
mklink /D ".git\hooks" "..\git_hooks"
if errorlevel 1 (goto fail) else (goto end)

:fail
echo Hook installation failed. Make sure you run this batch with admin privileges and that the folder doesnt exist.

:end
echo Done.
pause
