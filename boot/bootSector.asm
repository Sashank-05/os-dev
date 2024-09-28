; Section 1: BIOS Stage Setup
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

; Section 2: Print Loading Message
    mov si, loading_msg
    call print_string

; Section 3: Load Kernel from Disk
    mov ah, 0x02
    mov al, 1     ; Number of sectors to read (adjust as needed)
    mov ch, 0
    mov dh, 0
    mov cl, 2
    mov bx, kernel_offset
    int 0x13
    jc disk_error

    cmp al, 1  
    jne sectors_error
; Section 4: Print Success Message
    mov si, success_msg
    call print_string

; Section 5: Switch to Protected Mode
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:init_pm

; Section 6: Disk Error Handling
disk_error:
    mov si, disk_error_msg
    call print_string
    shr ax, 8
    mov bx, ax
    call print_hex_bios
    jmp $

sectors_error:
    mov si, SECTORS_ERROR_MSG
    call print_string
    mov dh, al
    call print_hex_bios      ; This is the number of sectors actually read
    jmp $

; Section 7: Utility Functions (16-bit mode)
print_hex_bios:
    push ax
    push bx
    push cx
    mov ah, 0x0E
    mov al, '0'
    int 0x10
    mov al, 'x'
    int 0x10
    mov cx, 4
print_hex_bios_loop:
    cmp cx, 0
    je print_hex_bios_end
    push bx
    shr bx, 12
    cmp bx, 10
    jge print_hex_bios_alpha
    mov al, '0'
    add al, bl
    jmp print_hex_bios_loop_end
print_hex_bios_alpha:
    sub bl, 10
    mov al, 'A'
    add al, bl
print_hex_bios_loop_end:
    int 0x10
    pop bx
    shl bx, 4
    dec cx
    jmp print_hex_bios_loop
print_hex_bios_end:
    pop cx
    pop bx
    pop ax
    ret

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

; Section 8: Protected Mode Initialization
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

; Section 9: Global Descriptor Table (GDT)
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

; Section 10: Utility Functions (32-bit mode)
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

; Section 11: Constants and Data
space_char equ ' '
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start
vga_start equ 0x000B8000
vga_extent equ 80 * 25 * 2
style_wb equ 0x0F

success_32_msg db 'Switched to 32-bit protected mode!', 0
loading_msg db 'Loading kernel...', 13, 10, 0
success_msg db 'switching to protected mode...', 13, 10, 0
disk_error_msg db 'Disk read error! Error code: ', 0
SECTORS_ERROR_MSG db "Incorrect number of sectors read. Sectors read: ", 0
kernel_offset equ 0x516

; Section 12: Bootloader Signature
times 510-($-$$) db 0
dw 0xAA55