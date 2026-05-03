@echo off
SETLOCAL EnableDelayedExpansion

set "VS_PATH=D:\Program Files\Microsoft Visual Studio\2022\Community"
call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"

set "PATH=%VS_PATH%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;%VS_PATH%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"

cd /d D:\code\StoneAgeClient

cmake --build out/build/x64-debug --config Debug

ENDLOCAL
