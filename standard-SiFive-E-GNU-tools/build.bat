@echo off
echo Cleaning up...

if exist *.o del /F /Q *.o
if exist *.elf del /F /Q *.elf

if "%1"=="clean" (
	echo OK
	exit /b 0
) 

echo Compiling...
%GCC_PREFIX%gcc -march=rv32i -O2 -I. -c -o startup.o startup.S
%GCC_PREFIX%gcc -march=rv32i -O2 -I. -c -o main.o main.c

echo Linking...
%GCC_PREFIX%ld -nostartfiles -T SiFive.ld -o fxrtos_demo.elf main.o startup.o libfxrtos.a
