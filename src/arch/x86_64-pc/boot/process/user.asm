[bits 64]

global enter_user_mode
enter_user_mode:
    ; rdi = rip, rsi = user rsp, rdx = cr3
    mov ax, 0x1B
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push qword 0x1B
    push rsi
    pushfq
    or qword [rsp], 0x200
    push qword 0x23
    push rdi
    
    ; Load CR3 as late as possible, right before iretq
    mov rax, rdx
    mov cr3, rax
    
    iretq
