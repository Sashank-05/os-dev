[bits 64]

section .data
align 0x10

gdt:
  null_descriptor:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 00000000b
    db 00000000b
    db 0x00

  kernel_code_64:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 10011010b
    db 00100000b
    db 0x00

  kernel_data:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 10010010b
    db 00000000b
    db 0x00

  user_data_64:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 11110010b
    db 00000000b
    db 0x00

  user_code_64:
    dw 0x0000
    dw 0x0000
    db 0x00
    db 11111010b
    db 00100000b
    db 0x00

align 0x10
tss:
  times 104 db 0
tss_end:

align 0x10
tss_descriptor:
  dq 0
  dq 0

gdt_end:

global gdt_ptr


section .text

gdt_ptr:
  dw gdt_end - gdt - 1
  dq gdt


CODE_SEG equ kernel_code_64 - gdt
DATA_SEG equ kernel_data - gdt
USER_DATA_SEG equ user_data_64 - gdt
USER_CODE_SEG equ user_code_64 - gdt
TSS_SEG equ tss_descriptor - gdt

global init_gdt
init_gdt:
  lgdt [gdt_ptr]
  mov ax, DATA_SEG
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax
  pop rdi
  push CODE_SEG
  push rdi
  retfq

global init_tss
init_tss:
  mov qword [tss + 4], rdi
  mov ax, tss_end - tss - 1
  mov word [tss_descriptor], ax
  mov rax, tss
  mov word [tss_descriptor + 2], ax
  shr rax, 16
  mov byte [tss_descriptor + 4], al
  mov byte [tss_descriptor + 5], 10001001b
  mov byte [tss_descriptor + 6], 0
  shr rax, 8
  mov byte [tss_descriptor + 7], al
  shr rax, 8
  mov dword [tss_descriptor + 8], eax
  mov dword [tss_descriptor + 12], 0
  mov ax, TSS_SEG
  ltr ax
  ret
