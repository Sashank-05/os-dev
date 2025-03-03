[org 0x7c00]

bits 16

draw_snake:
    mov ah, 0x0e
    mov al, "O"
    int 0x10
    ret

start:
    call draw_snake

times 510-($-$$) db 0
dw 0xaa55
    
    
    