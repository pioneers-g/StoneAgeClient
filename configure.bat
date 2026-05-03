@echo off
call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set VCPKG_ROOT=D:\code\vcpkg
cd /d D:\code\StoneAgeClient
cmake --preset x64-debug
