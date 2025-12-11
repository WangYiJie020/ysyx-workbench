ENTRY(_start)
PHDRS { text PT_LOAD; data PT_LOAD; }

SECTIONS {
  . = _mrom_start;
  .text : {
    *(entry)
    *(.text*)
  } : text
  etext = .;
  _etext = .;
  .rodata : {
    *(.rodata*)
  }
  .data : {
    *(.data*)
  } : data
  edata = .;
  _data = .;
  .bss : {
	_bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
  }

  _stack_top = _sram_start + _sram_size - _stack_size;
  . = _stack_top + _stack_size - 0x4;
  _stack_pointer = .;
  end = .;
  _end = .;
  _heap_start = _sram_start;
}
