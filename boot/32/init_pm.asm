[BITS 32]
global init_pm

init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp
    jmp begin_pm

begin_pm:
    call clear_protected
    mov esi, success_32_msg
    call print_protected

    mov esp, 0x90000
    jmp CODE_SEG:kernel_offset
