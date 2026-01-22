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

  /* .data : { */
		/* _data_start = .; */
  /*   *(.sdata*) */
  /*   *(.data*) */
		/* _data_end = .; */
  /* } > psram AT> flash */
  .data :
  {
    _data_start = .;
    *(.data*)
    *(.sdata*)

    /* for rt-thread */
    __fsymtab_start = .;
    KEEP(*(FSymTab))
    __fsymtab_end = .;
    . = ALIGN(8);
    __vsymtab_start = .;
    KEEP(*(VSymTab))
    __vsymtab_end = .;
    . = ALIGN(8);

    . = ALIGN(8);
    __rt_init_start = .;
    KEEP(*(SORT(.rti_fn*)))
    __rt_init_end = .;
    . = ALIGN(8);

    __rt_utest_tc_tab_start = .;
    KEEP(*(UtestTcTab))

    . = ALIGN(8);
    __am_apps_data_start = .;
    *(__am_apps.data*)
    *(__am_apps.sdata*)
    __am_apps_data_end = .;
    . = ALIGN(8);
    __rt_utest_tc_tab_end = .;
    /* end for rt-thread */
	} > psram AT> flash
  _data_end = ADDR(.bss);
  _data_load_start = LOADADDR(.data);
  .bss :
  {
    _bss_start = .;
    *(.bss*)
    *(.sbss*)
    *(.scommon)
		_bss_end = .;
  } > psram AT> flash
  .bss.extra :
  {
    __am_apps_bss_start = .;
    *(__am_apps.bss*)
    *(__am_apps.sbss*)
    *(__am_apps.scommon*)
    __am_apps_bss_end = .;
  } > psram AT> flash

	__data_load_start__ = LOADADDR(.data); /* MA */
	__data_size__       = SIZEOF(.data);   /* LEN */

  /* .bss : { */
		/* _bss_start = .; */
  /*   *(.bss*) */
  /*   *(.sbss*) */
  /*   *(.scommon) */
		/* _bss_end = .; */
  /* } > psram */


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
