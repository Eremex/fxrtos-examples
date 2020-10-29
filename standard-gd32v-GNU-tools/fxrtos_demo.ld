
MEMORY
{
	rom (rx) : ORIGIN = 0x08000000, LENGTH = 128K
	ram (rwx) : ORIGIN = 0x20000000, LENGTH = 32K
}

ENTRY(reset_handler)

SECTIONS
{
	.reset : {
		*(.reset)
		*(.vectors)
	} >rom
        
    .init : {
		*(.init)
	} >rom

	.text : {
		*(.text*)
		. = ALIGN(4);
		*(.rodata*)
		*(.srodata*)
		. = ALIGN(4);
	} >rom

	. = ALIGN(4);
	_etext = .;

	.data : {
		_data = .;
		*(.data*)
		*(.sdata*)
	} >ram AT >rom
	_data_loadaddr = LOADADDR(.data);

	. = ALIGN(4);
	_edata = .;

	.bss : {
                _bss = .;
		*(.bss*)
		*(.sbss*)
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
	} >ram

	. = ALIGN(4);
	end = .;
}
