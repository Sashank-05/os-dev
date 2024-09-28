/* Linker script for the kernel */
OUTPUT_FORMAT(elf32-i386)
ENTRY(kernel_main)

SECTIONS
{
    . = 0x516;  /* This is where we told the bootloader to load our kernel */

    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }
}