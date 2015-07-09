@echo off
REM === pause if there were errors ===
if %errorlevel% neq 0 pause
@echo on
mkdir brahma_loader\data
mkdir output
make
cd brahma_loader
make
move Decrypt9.* ..\output
cd ..
rm output/Decrypt9.elf
pause