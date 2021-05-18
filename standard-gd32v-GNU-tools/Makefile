#
# Makefile for FX-RTOS demo.
#

GCC_PREFIX ?= riscv64-elf-
CC=$(GCC_PREFIX)gcc

src_c=$(wildcard *.c)
src_S=$(wildcard *.S)

ASFLAGS=-g -march=rv32im -mabi=ilp32 -I.
CFLAGS = -ffreestanding -march=rv32im -mabi=ilp32 -O2 -I. -DUSE_STDPERIPH_DRIVER
LDFLAGS = -nostdlib -lgcc -nostartfiles -Xlinker -melf32lriscv -Xlinker -Map=output.map -T fxrtos_gd32v.ld libfxrtos.a

all: fxrtos_demo.elf fxrtos_demo.bin

objs = $(src_S:.S=.o) $(src_c:.c=.o)

fxrtos_demo.elf: $(objs)
	$(CC) -o $@ $^ $(LDFLAGS)

fxrtos_demo.bin: fxrtos_demo.elf
	$(GCC_PREFIX)objcopy -O binary $^ $@

.PHONY: clean
clean:
	rm -f $(objs)

