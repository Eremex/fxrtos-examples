del /F /Q *.o *.elf *.hex 2>nul

%GCC_PREFIX%gcc -mcpu=cortex-m3 -mthumb -c -o boot.o boot.S
%GCC_PREFIX%gcc -ffreestanding -mcpu=cortex-m3 -O2 -DSTM32F103xB -mthumb -I . -c -o main.o main.c
%GCC_PREFIX%gcc -ffreestanding -mcpu=cortex-m3 -O2 -DSTM32F103xB -mthumb -I . -c -o system_stm32f1xx.o system_stm32f1xx.c

%GCC_PREFIX%ld -nostartfiles -T link.lds -o fxrtos_demo.elf boot.o system_stm32f1xx.o main.o libfxrtos.a
%GCC_PREFIX%objcopy -O binary fxrtos_demo.elf fxrtos_demo.bin
