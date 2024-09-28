[BITS 16]
global load_kernel

load_kernel:
    ; Load kernel from disk
    mov ah, 0x02
    mov al, 2      ; Number of sectors to read (adjust as needed)
    mov ch, 0
    mov dh, 0
    mov cl, 2
    mov bx, kernel_offset
    int 0x13
    jc disk_error

    ret
disk_error:
    mov si, disk_error_msg
    call print_string
    jmp $
