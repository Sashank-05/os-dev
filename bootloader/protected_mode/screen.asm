[bits 32]
clear_protected:
    pushad
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
    add edx,2
    jmp clear_protected_loop

clear_protected_done:
    popad
    ret

space_char:                     equ ` `


[bits 32]
print_protected:
    pushad
    mov edx, vga_start

    print_protected_loop:
        cmp byte[esi], 0
        je  print_protected_done

        mov al, byte[esi]
        mov ah, style_wb
        mov word[edx], ax

        add esi, 1
        add edx, 2

        jmp print_protected_loop

print_protected_done:
    popad
    ret
