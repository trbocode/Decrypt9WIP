@echo off
REM === pause if there were errors ===
if %errorlevel% neq 0 pause
@echo on
mkdir output
make
cd brahma_loader
mkdir data
make
move Decrypt9.* ..\output
cd ..
pause