/*
 *  FX-RTOS demo linker script for GNU toolchain.
 */

MEMORY
{
     FLASH (rx) : ORIGIN = 0x8000000 LENGTH = 0x20000
     SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 0x8000
}

ENTRY(RESET_Handler)

SECTIONS
{
	.text :
	{
	  *(.vectors*)
	  *(.text*)
	  *(.rodata*)
	  _etext = .;
	} > FLASH

	.data : AT (_etext)
	{
          _data = .;
	  *(.data*)
	  _edata = .;
	} > SRAM

	.bss :
	{
	  _bss = .;
	  *(.bss*)
	  *(COMMON)
	  _ebss = .;
	} > SRAM
	
	_data_sz   = (_edata - _data);
	_bss_sz   = _ebss - _bss;	
}
