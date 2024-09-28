[BITS 32]
global print_protected

print_protected:
    pusha
    mov edx, vga_start

print_protected_loop:
    cmp byte[esi], 0
    je print_protected_done

    mov al, byte[esi]
    mov ah, 0x0F

    mov word[edx], ax

    add esi, 1
    add edx, 2

    jmp print_protected_loop

print_protected_done:
    popa
    ret
