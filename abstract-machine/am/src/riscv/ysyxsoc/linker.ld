ENTRY(_start)
/* PHDRS { text PT_LOAD; data PT_LOAD; } */

MEMORY {
  mrom : ORIGIN = 0x20000000, LENGTH = 4K
  sram : ORIGIN = 0x0f000000, LENGTH = 8K
	flash : ORIGIN = 0x30000000, LENGTH = 128K
	psram : ORIGIN = 0x80000000, LENGTH = 2M
}

SECTIONS {
	. = ORIGIN(flash);
  .text : {
		_text = .;
    *(entry)
    *(.text*)
		_etext = .;
  } > flash
  /* .rodata : { */
  /*   *(.rodata*) */
  /* } > mrom AT> mrom */
  .data : {
		_data = .;
    *(.sdata*)
    *(.data*)
		_edata = .;
  } > psram AT> flash

	__data_load_start__ = LOADADDR(.data); /* MA */
	__data_size__       = SIZEOF(.data);   /* LEN */

  .bss : {
		_bss = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
		_ebss = .;
  } > psram

	/* _sram_start = ORIGIN(sram); */
	/* _sram_size = LENGTH(sram); */

	_psram_start = ORIGIN(psram);
	_psram_size = LENGTH(psram);

  _stack_top = _psram_start + _psram_size - _stack_size;
  _stack_pointer  = _stack_top + _stack_size;

  _heap_start = _ebss;
	_heap_end = _stack_top;
}
