[BITS 32]
global clear_protected

clear_protected:
    pusha
    mov ebx, vga_extent
    mov ecx, vga_start
    mov edx, 0
clear_protected_loop:
    cmp edx, ebx
    jge clear_protected_done

    push edx

    mov al, space_char
    mov ah, style_wb

    add edx, ecx
    mov word[edx], ax

    pop edx

    add edx, 2
    jmp clear_protected_loop

clear_protected_done:
    popa
    ret

space_char equ ' '
