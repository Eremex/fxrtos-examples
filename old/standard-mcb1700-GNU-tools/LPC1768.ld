
MEMORY
{
     FLASH (rx) : ORIGIN = 0x0 LENGTH = 0x80000
     SRAM (rwx) : ORIGIN = 0x10000000, LENGTH = 0x8000
	 AHBRAM0   : ORIGIN = 0x2007c000, LENGTH = 0x4000
	 AHBRAM1   : ORIGIN = 0x20080000, LENGTH = 0x4000
}

_vRamTop = 0x10000000 + 0x8000;

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
		*(vtable)
		*(.data*)
		_edata = .;
	} > SRAM

	/* zero initialized data */
	.bss :
	{
		_bss = .;
		*(.bss*)
		*(COMMON)
		_ebss = .;
	} > SRAM
	
	/* Where we put the heap with cr_clib */
	.cr_heap : 
	{
		end = .;
		_pvHeapStart = .;
	} > SRAM

/*
	Note: (ref: M0000066)
	Moving the stack down by 16 is to work around a GDB bug.
	This space can be reclaimed for Production Builds.
*/	
	_vStackTop = _vRamTop - 16;
	_data_sz   = (_edata - _data);
	_bss_sz   = _ebss - _bss;
	
}
