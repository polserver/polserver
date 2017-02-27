@echo off


if exist boost_1_63_0\boost goto dontextract


..\pol-core\dist\7za.exe x -so boost_1_63_0.tar.bz2 | ..\pol-core\dist\7za.exe x -si -ttar

:dontextract
echo Nothing to do. Files already extracted.

