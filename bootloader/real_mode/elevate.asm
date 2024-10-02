[bits 16]

elevate_bios:
    ; change vga mode
    ;mov ah, 0x00
    ;mov al, 0x13
    int 0x10
    cli
    lgdt [gdt_32_descriptor]
    mov eax, cr0
    or eax, 0x00000001
    mov cr0, eax
    jmp code_seg:init_pm

    [bits 32]
    init_pm:
    mov ax, data_seg
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x90000
    mov esp, ebp
    jmp begin_protected
