OUTPUT_FORMAT("binary")

SECTIONS
{
    . = 0x7C00;                /* Kernel starts at 0x1000 */
    .text :
    {
        *(.text)               /* Text section contains the code */
    }
    .data :
    {
        *(.data)               /* Data section */
    }
    .bss :
    {
        *(.bss)                /* BSS section (uninitialized data) */
    }
}
