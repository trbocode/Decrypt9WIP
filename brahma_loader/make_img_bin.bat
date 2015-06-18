@echo off

convert %1 -rotate 90 %~n1.bgr
ren %~n1.bgr %~n1.bin
mv %~n1.bin gfx
echo DONE!
pause
