@echo off
call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >/dev/null 2>&1
cd /d D:\code\StoneAgeClient
out\build\x64-debug\tests\test_sprite_blend_comprehensive.exe > D:\code\StoneAgeClient\test_r7.txt 2>&1
echo ===BLEND=== >> D:\code\StoneAgeClient\test_r7.txt
out\build\x64-debug\tests\test_critical_stubs.exe >> D:\code\StoneAgeClient\test_r7.txt 2>&1
echo ===CRITICAL=== >> D:\code\StoneAgeClient\test_r7.txt
out\build\x64-debug\tests\test_stubs_round3.exe >> D:\code\StoneAgeClient\test_r7.txt 2>&1
echo ===ROUND3=== >> D:\code\StoneAgeClient\test_r7.txt
out\build\x64-debug\tests\test_map_tileattr_comprehensive.exe >> D:\code\StoneAgeClient\test_r7.txt 2>&1
echo ===TILEATTR=== >> D:\code\StoneAgeClient\test_r7.txt
