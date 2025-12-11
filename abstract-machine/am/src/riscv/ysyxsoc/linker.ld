ENTRY(_start)
/* PHDRS { text PT_LOAD; data PT_LOAD; } */

MEMORY {
  mrom : ORIGIN = 0x20000000, LENGTH = 4K
  sram : ORIGIN = 0x0f000000, LENGTH = 8K
}

SECTIONS {
  . = ORIGIN(mrom);
  .text : {
		_text = .;
    *(entry)
    *(.text*)
		_etext = .;
  } > mrom AT> mrom 
  /* .rodata : { */
  /*   *(.rodata*) */
  /* } > mrom AT> mrom */
  .data : {
		_data = .;
    *(.data*)
		_edata = .;
  } > sram AT> mrom

  .bss : {
	_bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  } > sram

	_sram_start = ORIGIN(sram);
	_sram_size = LENGTH(sram);

  _stack_top = _sram_start + _sram_size - _stack_size;
  . = _stack_top + _stack_size;
  _stack_pointer = .;
  end = .;
  _end = .;
  _heap_start = _edata;
	_heap_end = _stack_top;
}
