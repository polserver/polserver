@echo off

echo This script will extract boost to this folder.
pause

..\pol-core\dist\7za.exe x -so boost_1_54_0.tar.bz2 | ..\pol-core\dist\7za.exe x -si -ttar

