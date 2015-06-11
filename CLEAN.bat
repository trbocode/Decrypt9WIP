@echo off
rm brahma_loader/data/payload.bin
rm *.elf
make clean
cd brahma_loader
make clean
pause