@echo off
echo Cleaning up...

if exist *.o del /F /Q *.o
if exist *.elf del /F /Q *.elf
if exist *.hex del /F /Q *.hex

if "%1"=="clean" (
	echo OK
	exit /b 0
) 

call set OBJS=

echo Compiling...

for %%f in (*.c) do (
	echo %%f
	%GCC_PREFIX%gcc -mcpu=cortex-m3 -mthumb -I. -c -o %%~nf.o %%f
	call set OBJS=%%OBJS%% %%~nf.o
)

for %%f in (*.S) do (
	echo %%f
	%GCC_PREFIX%gcc -mcpu=cortex-m3 -mthumb -I. -c -o %%~nf.o %%f
	call set OBJS=%%OBJS%% %%~nf.o
)

echo Linking...

%GCC_PREFIX%ld -u _vectors --Map=map.txt --gc-sections -nostartfiles -T LPC1768.ld -o fxrtos_demo.elf %OBJS% libfxrtos.a

%GCC_PREFIX%objcopy --target ihex fxrtos_demo.elf fxrtos_demo.hex

echo Done.

