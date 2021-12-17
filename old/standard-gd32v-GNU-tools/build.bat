@echo off
echo Cleaning up...

if exist *.o del /F /Q *.o
if exist *.elf del /F /Q *.elf

if "%1"=="clean" (
	echo OK
	exit /b 0
) 

echo Compiling...
%GCC_PREFIX%gcc -g -DGD32VF103V_EVAL -march=rv32i -O2 -ffunction-sections -I. -c -o fxrtos_gd32v_startup.o fxrtos_gd32v_startup.s
%GCC_PREFIX%gcc -g -DUSE_STDPERIPH_DRIVER  -march=rv32im -O2 -ffunction-sections -I. -c -o main.o main.c
%GCC_PREFIX%gcc -g -DUSE_STDPERIPH_DRIVER  -march=rv32im -O2 -ffunction-sections -I. -c -o system_gd32vf103.o system_gd32vf103.c
%GCC_PREFIX%gcc -g -DUSE_STDPERIPH_DRIVER  -march=rv32im -O2 -ffunction-sections -I. -c -o gd32vf103_rcu.o gd32vf103_rcu.c

echo Linking...
%GCC_PREFIX%ld -gc-sections -nostartfiles -T fxrtos_gd32v.ld -o fxrtos_demo.elf main.o fxrtos_gd32v_startup.o system_gd32vf103.o gd32vf103_rcu.o libfxrtos.a
%GCC_PREFIX%objcopy --target ihex fxrtos_demo.elf fxrtos_demo.hex