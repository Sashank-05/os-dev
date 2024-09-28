[BITS 32]
global setup_gdt

setup_gdt:
    lgdt [gdt_descriptor]
    ret

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

