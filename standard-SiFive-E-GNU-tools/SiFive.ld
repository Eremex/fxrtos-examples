OUTPUT_ARCH( "riscv" )
ENTRY(_reset_vector)

MEMORY
{
    xip (rx) : ORIGIN = 0x20400000, LENGTH = 512k
	sram (rw) : ORIGIN = 0x80000000, LENGTH = 512k
}

SECTIONS
{
	.text : 
	{
		. = ALIGN(0x10);
		*(.entry);
		. = ALIGN(0x10);
		*(.text*);
		*(.rodata*);
	} > xip

	.bss :
	{
		_bss = .;
		*(.bss*);
		*(.sbss*);
		_ebss = .;
	} > sram
	
	_bss_sz   = _ebss - _bss;
}
