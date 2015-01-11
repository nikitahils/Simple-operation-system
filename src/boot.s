[BITS 32]
[GLOBAL start]
[GLOBAL multiboot_header]
[EXTERN loader_main]
					; GNU Multiboot Specification version 0.6.96
					; http://www.gnu.org/software/grub/manual/multiboot/multiboot.html
								  ;
MBOOT_HEADER_MAGIC  equ 0x1BADB002 				  ; Multiboot Magic value
MBOOT_PAGE_ALIGN    equ 1<<0    				  ; Load kernel and modules on a page boundary
MBOOT_MEM_INFO      equ 1<<1    				  ; Provide your kernel with memory info
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

multiboot_header:			; multiboot.h
					;
				    	; struct multiboot_header {
	dd  MBOOT_HEADER_MAGIC      	; 	multiboot_uint32_t magic;
    	dd  MBOOT_HEADER_FLAGS      	; 	multiboot_uint32_t flags;
    	dd  MBOOT_CHECKSUM          	;  	multiboot_uint32_t checksum;
					;       ...
				    	; };
start:
	push eax 			; Must contain the magic value ‘0x2BADB002’
					; the presence of this value indicates to the operating system that it was loaded by a Multiboot-compliant boot loader
	push ebx			; Must contain the 32-bit physical address of the Multiboot information structure provided by the boot loader
	call loader_main
	jmp $
