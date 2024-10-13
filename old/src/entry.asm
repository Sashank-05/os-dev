section .multiboot_header

mb_start:
align 8

; ASM macros

MAGIC_NUMBER      equ 0xe85250d6  	; multiboot2 magic 
FLAGS             equ 
HEADER_LEN	  equ mb_end-mb_start
CHECKSUM          equ -(MAGIC_NUMBER + FLAGS + HEADER_LEN)

; Multiboot 2 header, according to specification (16bytes)

  dd MAGIC_NUMBER		; dd = 4 bytes = 32bits = u32
  dd FLAGS			
  dd HEADER_LEN
  dd CHECKSUM

; Tags? (28bytes)

; Tag 1 : set graphics mode (only recommended, can be overriden by GRUB)
  align 8
  dw 5			; 2
  dw 0			; 2
  dd 20			; 4
  dd 1200		; 4
  dd 768		; 4
  dd 32			; 4

; End of tags

  align 8
  dw 0			; 2
  dw 0			; 2
  dd 8			; 4

  mb_end:

section .text:

KERNEL_STACK_SIZE equ 4096
extern kernel_main

loader:
	mov esp, kernel_stack + KERNEL_STACK_SIZE
  	cli
  	push rbx
  	call kernel_main

.loop:
  	jmp .loop

kernel_stack:
	resb KERNEL_STACK_SIZE