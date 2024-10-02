[org 0x7C00]

jmp begin_real

kernel_size db 0

begin_real:

[bits 16]

mov bp, 0x0500
mov sp, bp

mov byte [boot_drive], dl

mov bx, msg_hello_world
call print_bios

mov bx, 0x0002

mov cx, [kernel_size]
add cx, 2

mov dx, 0x7E00

call load_bios

call elevate_bios

bootsector_hold:
jmp $

%include "real_mode/print.asm"
%include "real_mode/print_hex.asm"
%include "real_mode/load.asm"
%include "real_mode/gdt.asm"
%include "real_mode/elevate.asm"

msg_hello_world: db `\r\nHello World, from the BIOS!\r\n`, 0

boot_drive: db 0x00

times 510 - ($ - $$) db 0x00  ; Fill remaining bytes to reach 512 bytes

dw 0xAA55                     ; Boot sector signature

bootsector_extended:

begin_protected:

[bits 32]
call clear_protected

;call detect_lm_protected

mov esi, protected_alert
call print_protected
mov esi, 0

;call init_pt_protected

call kernel_mode


jmp $

%include "protected_mode/screen.asm"
%include "protected_mode/init_pt.asm"
%include "protected_mode/detect_lm.asm"

vga_start: equ 0x000B8000
vga_extent: equ 80 * 25 * 2
style_wb: equ 0x0F

protected_alert: db `64-bit long mode supported`, 0

times 512 - ($ - bootsector_extended) db 0x00  ; Padding to ensure 512-byte sector size
kernel_mode:

call kernel_start

kernel_start: equ 0x8200
times 512 - ($ - kernel_mode) db 0x00          ; Ensure each segment aligns correctly
