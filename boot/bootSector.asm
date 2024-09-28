[BITS 16]
[ORG 0x7C00]

global start
extern main
start:
    ; Set up segments and stack
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x9000

    ; Print loading message
    mov si, loading_msg
    call print_string

    ; Load kernel from disk
    mov ah, 0x02
    mov al, 2      ; Number of sectors to read (adjust as needed)
    mov ch, 0
    mov dh, 0
    mov cl, 2
    mov bx, kernel_offset
    int 0x13
    jc disk_error

    ; Print success message
    mov si, success_msg
    call print_string

    ; Switch to protected mode
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:init_pm

disk_error:
    mov si, disk_error_msg
    call print_string
    jmp $

print_string:
    push ax
    push bx
    push cx
    push dx
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    pop dx
    pop cx
    pop bx
    pop ax
    ret


    
[BITS 32]
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

gdt_start:
    dq 0x0
gdt_null:
    dd 0x00000000           ; All values in null entry are 0
    dd 0x00000000   
gdt_code:
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10011010b
    db 11001111b
    db 0x0
gdt_data:
    dw 0xFFFF
    dw 0x0
    db 0x0
    db 10010010b
    db 11001111b
    db 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[bits 32]
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

[bits 32]
print_protected:

    pusha
    mov edx, vga_start

    ; Do main loop
    print_protected_loop:

        cmp byte[esi], 0
        je  print_protected_done


        mov al, byte[esi]
        mov ah, 0x0F

        mov word[edx], ax

        add esi, 1
        add edx, 2

        jmp print_protected_loop

print_protected_done:
    popa
    ret

space_char:                     equ ` `

CODE_SEG: equ gdt_code - gdt_start
DATA_SEG: equ gdt_data - gdt_start

vga_start:                  equ 0x000B8000
vga_extent:                 equ 80 * 25 * 2 
style_wb:                   equ 0x0F

success_32_msg db 'Switched to 32-bit protected mode!', 0
loading_msg db 'Loading kernel...', 13, 10, 0
success_msg db 'Kernel loaded successfully, switching to protected mode...', 13, 10, 0
disk_error_msg db 'Disk read error!', 13, 10, 0

kernel_offset equ 0x1000

times 510-($-$$) db 0
dw 0xAA55

