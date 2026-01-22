ENTRY(_start)
/* PHDRS { text PT_LOAD; data PT_LOAD; } */

MEMORY {
  mrom : ORIGIN = 0x20000000, LENGTH = 4K
  sram : ORIGIN = 0x0f000000, LENGTH = 8K
	flash : ORIGIN = 0x30000000, LENGTH = 1M
	psram : ORIGIN = 0x80000000, LENGTH = 4M
}

SECTIONS {
	. = ORIGIN(flash);

	.boot : {
		_boot_start = .;
		*(entry)
		*(.boot_text)
		_boot_end = .;
	} > flash AT> flash

	.psram_padding (NOLOAD) : {
			. = ORIGIN(psram) + SIZEOF(.boot) - 12;
	} > psram

  .text : {
		_text_start = .;
    *(.text*)
		_text_end = .;
  } > psram AT> flash

	__text_load_start__ = LOADADDR(.text); /* flash addr */
	__text_size__       = SIZEOF(.text);   /* LEN */

  .rodata : {
		_rodata_start = .;
    *(.rodata*)
		*(.srodata*)
		_rodata_end = .;
  } > psram AT> flash

	__rodata_load_start__ = LOADADDR(.rodata);
	__rodata_size__       = SIZEOF(.rodata);

  .data : {
		_data_start = .;
    *(.sdata*)
    *(.data*)
		_data_end = .;
  } > psram AT> flash

	__data_load_start__ = LOADADDR(.data); /* MA */
	__data_size__       = SIZEOF(.data);   /* LEN */

  .bss : {
		_bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
		_bss_end = .;
  } > psram


	__sram_start__ = ORIGIN(sram);
	__sram_size__ = LENGTH(sram);
	__sram_end__ = __sram_start__ + __sram_size__;

	__psram_start__ = ORIGIN(psram);
	__psram_size__ = LENGTH(psram);
	__psram_end__ = __psram_start__ + __psram_size__;

  _stack_top = __sram_end__;
  _stack_pointer  = _stack_top;

  _heap_start = _bss_end;
	_heap_end = __psram_end__;
}
