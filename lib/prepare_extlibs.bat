@echo off


if exist boost_1_63_0\boost goto extract_curl
..\pol-core\dist\7za.exe x -so boost_1_63_0.tar.bz2 | ..\pol-core\dist\7za.exe x -si -ttar

:extract_curl
if exist curl-7.57.0 goto done
..\pol-core\dist\7za.exe x curl-7.57.0.zip
cp Makefile-libcurl-polserver.vc curl-7.57.0\winbuild

:done
echo Finished extracting files.