#
# Makefile for FX-RTOS demo.
#

GCC_PREFIX ?= riscv64-elf-

%.o : %.c
	$(GCC_PREFIX)gcc -march=rv32im -mabi=ilp32 -O2 -DUSE_STDPERIPH_DRIVER -ffreestanding -I . -c -o $@ $<

%.o : %.S
	$(GCC_PREFIX)gcc -march=rv32im -mabi=ilp32 -c -o $@ $<

demo : fxrtos_gd32v_startup.o system_gd32vf103.o gd32vf103_rcu.o main.o
	$(GCC_PREFIX)ld -melf32lriscv -T fxrtos_gd32v.ld -o fxrtos_demo.elf fxrtos_gd32v_startup.o system_gd32vf103.o gd32vf103_rcu.o main.o libfxrtos.a
	$(GCC_PREFIX)objcopy -O binary fxrtos_demo.elf fxrtos_demo.bin

clean:
	rm -f *.o *.elf *.bin

