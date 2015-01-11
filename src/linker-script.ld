ENTRY(start)
lma             = 1M;
null_ptr_offset = 0x20;

SECTIONS
{

/*-----------------------------------------------*/
/*------------------kernel loader----------------*/
/*-----------------------------------------------*/
/*-----------------------------------------------*/

	. = lma;
	.text : {
		./output/objects/boot.o(.text)
		./output/objects/loader.o(.text)
	}

	.data : {
		./output/objects/boot.o(.data)
		./output/objects/loader.o(.rodata)
	}

	.bss : {
		./output/objects/boot.o(COMMON)
		./output/objects/boot.o(.bss)
		./output/objects/loader.o(.bss)
		./output/objects/loader.o(COMMON)
	}
	. = ALIGN(4K);
	loader_size = . - lma;

/*-----------------------------------------------*/
/*--------------kernel code segment--------------*/
/*-----------------------------------------------*/
/*-----------------------------------------------*/

	. = null_ptr_offset;
	.kernel_code : AT(lma + loader_size) {
		*(.text)
	}
	. = ALIGN(4K);
	kernel_code_size = .;

/*-----------------------------------------------*/
/*--------------kernel data segment--------------*/
/*-----------------------------------------------*/
/*-----------------------------------------------*/

    . = null_ptr_offset;
	.kernel_data : AT(lma + loader_size + kernel_code_size) {
		*(.data)
		*(.rodata)
		*(.bss)
		*(COMMON)
	}
	. = ALIGN(4K);
	kernel_data_size = .;

/*-----------------------------------------------*/
/*--------------info variables-------------------*/
/*-----------------------------------------------*/
/*-----------------------------------------------*/

    kernel_size = loader_size + kernel_code_size + kernel_data_size;

/*-----------------------------------------------*/
/*-------------discarded sections----------------*/
/*-----------------------------------------------*/
/*-----------------------------------------------*/

	/DISCARD/ : {
		*(.comment)
	}

}
