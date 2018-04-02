@echo off

if "%POLARCH%" == "x86" (
  set ARCHBIT=32
)
if "%POLARCH%" == "x64" (
  set ARCHBIT=64
)
if "%POLARCH%" == "" (
  set POLARCH=x64
  set ARCHBIT=64
)

cmake .. -DCMAKE_BUILD_TYPE=Release -DFORCE_ARCH_BITS="%ARCHBIT%" -DCMAKE_GENERATOR_PLATFORM=%POLARCH%
cmake --build . --target PolRelease --config Release -- /m /p:Platform="%POLARCH%" /p:Configuration="Release" /v:m
pause
goto exit

:exit