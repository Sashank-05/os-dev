; SAS OS Bootloader
; Author: Sashank Vanka

[org 0x7C00]

jmp real
kernel_size db 0

[bits 16]
real:
    mov bp, 0x0500              ; Set up base pointer
    mov sp, bp                  ; Set stack pointer
    mov byte [drive], dl   ; Store boot drive number
    mov bx, 0x0002              ; Starting sector to load from
    mov cx, [kernel_size]       ; Get kernel size
    add cx, 2                   ; Adjust size for sectors
    mov dx, 0x7E00              ; Address to load the kernel
    call load_kernel            ; Load kernel
    call elevate                ; Enter protected mode

bootsector_hold:
    jmp $                       ; Infinite loop to hold execution

load_kernel:
    push ax
    push bx
    push cx
    push dx
    push cx                     ; Save registers
    mov ah, 0x02                ; BIOS read sectors
    mov al, cl                  ; Number of sectors to read
    mov cl, bl                  ; Sector number
    mov bx, dx                  ; Memory address to load
    mov ch, 0x00                ; Cylinder number
    mov dh, 0x00                ; Head number
    mov dl, byte [drive]   ; Boot drive
    int 0x13                    ; BIOS interrupt for disk services
    jc disk_error          ; Jump if there's an error
    pop bx
    cmp al, bl
    jne disk_error         ; Error if read doesn't match
    pop dx
    pop cx
    pop bx
    pop ax
    ret

disk_error:
    jmp $                       ; Infinite loop in case of error

gdt_start:
gdt_null:                       ; GDT null descriptor
    dd 0x00000000               ; Null descriptor
    dd 0x00000000               ; Null descriptor

gdt_code:                       ; GDT code segment descriptor
    dw 0xFFFF                   ; Segment limit (bits 0-15)
    dw 0x0000                   ; Base address (bits 0-15)
    db 0x00                     ; Base address (bits 16-23)
    db 0b10011010               ; Code segment flags (readable, non-conforming, accessed)
    db 0b11001111               ; Code segment limit flags (4K granularity, 32-bit mode)
    db 0x00                     ; Base address (bits 24-31)

gdt_data:                       ; GDT data segment descriptor
    dw 0xFFFF                   ; Segment limit (bits 0-15)
    dw 0x0000                   ; Base address (bits 0-15)
    db 0x00                     ; Base address (bits 16-23)
    db 0b10010010               ; Data segment flags (writable, expand-down, accessed)
    db 0b11001111               ; Data segment limit flags (4K granularity, 32-bit mode)
    db 0x00                     ; Base address (bits 24-31)

gdt_end:                        ; End of GDT
gdt_descriptor:                 ; GDT descriptor
    dw gdt_end - gdt_start - 1  ; GDT size
    dd gdt_start                ; GDT base address

code_seg: equ gdt_code - gdt_start
data_seg: equ gdt_data - gdt_start

elevate:
    mov ah, 0x00                ; ax = 0x0013 (VGA mode)
    mov al, 0x13                ; Set video mode before we switch to protected mode
    int 0x10                    ; screen interrupt
    cli                         ; Disable interrupts
    lgdt [gdt_descriptor]       ; Load GDT
    mov eax, cr0                ; Enable protected mode 
    or eax, 0x00000001          ; Set PE bit to enable protected mode
    mov cr0, eax                ; Update control register
    jmp code_seg:init_pm        ; Far jump to protected mode

[bits 32]
init_pm:
    mov ax, data_seg            ; Set data segment registers
    mov ds, ax                  ; Set data segment
    mov ss, ax                  ; Set stack segment
    mov es, ax                  ; Set extra segment
    mov fs, ax                  ; Set fs segment
    mov gs, ax                  ; Set gs segment
    mov ebp, 0x90000            ; Set base pointer
    mov esp, ebp                ; Set stack pointer
    jmp begin_protected         ; Jump to protected mode execution

drive: db 0x00

times 510 - ($ - $$) db 0x00    ; Pad to 510 bytes
dw 0xAA55                       ; Boot signature

bootsector_extended:            ; Extended boot sector
begin_protected:                ; Entry point for protected mode
    [bits 32]                                    
    call kernel_mode            ; Call kernel mode
    jmp $                       ; Never reach this point

times 512 - ($ - bootsector_extended) db 0x00   ; Pad to 512 bytes

kernel_mode:
    call kernel_start           ; Call kernel start address

kernel_start: equ 0x8200        ; Kernel start address

times 512 - ($ - kernel_mode) db 0x00           ; Pad to next sector
