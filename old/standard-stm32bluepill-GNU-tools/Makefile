#
# Makefile for FX-RTOS demo.
#

GCC_PREFIX ?= arm-none-eabi-

%.o : %.c
	$(GCC_PREFIX)gcc -ffreestanding -mcpu=cortex-m3 -O2 -DSTM32F103xB -mthumb -I . -c -o $@ $<

%.o : %.S
	$(GCC_PREFIX)gcc -mcpu=cortex-m3 -mthumb -c -o $@ $<

demo : boot.o system_stm32f1xx.o main.o
	$(GCC_PREFIX)ld -nostartfiles -T link.lds -o fxrtos_demo.elf boot.o system_stm32f1xx.o main.o libfxrtos.a
	$(GCC_PREFIX)objcopy -O binary fxrtos_demo.elf fxrtos_demo.bin

clean:
	rm -f *.o *.elf *.bin

