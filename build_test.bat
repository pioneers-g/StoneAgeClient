@echo off
call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >NUL 2>&1
cd /d D:\code\StoneAgeClient
set "PATH=D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;D:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;%PATH%"
cmake --preset x64-debug >NUL 2>&1
cmake --build out/build/x64-debug --config Debug > D:\code\StoneAgeClient\test_result.txt 2>&1
echo ===BUILD DONE=== >> D:\code\StoneAgeClient\test_result.txt
out\build\x64-debug\tests\test_critical_stubs.exe >> D:\code\StoneAgeClient\test_result.txt 2>&1
echo ===ROUND3 TESTS=== >> D:\code\StoneAgeClient\test_result.txt
out\build\x64-debug\tests\test_stubs_round3.exe >> D:\code\StoneAgeClient\test_result.txt 2>&1
