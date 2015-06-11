@echo off
REM === pause if there were errors ===
if %errorlevel% neq 0 pause
@echo on
make
cd Decrypt9
make
cd ..
mkdir output
move Decrypt9/Decrypt9.3dsx output
move Decrypt9/Decrypt9.smdh output
pause