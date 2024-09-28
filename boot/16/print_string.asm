[BITS 16]
global print_string

print_string:
    push ax
    push bx
    push cx
    push dx
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    pop dx
    pop cx
    pop bx
    pop ax
    ret
