section .data
    msg db 'Hello, World!', 0

section .text
    global _start

_start:
    ; Set up stack pointer in real mode
    mov esp, cr4

    ; Enable Protected Mode (switch to protected mode)
    cli          ; disable interrupts
    mov eax, cr0
    or eax, 1   ; bit 0 set = enable P-mode
    mov eax, cr4
    xor eax, eax ; clear all flags
    push eax      ; save current flags
    mov eax, cr4
    popfd        ; restore saved flags
    push eax

    ; Save current state of CR0 and CR3
    mov eax, cr0
    push eax
    mov eax, cr3
    push eax
