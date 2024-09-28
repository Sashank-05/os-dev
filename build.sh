#!/bin/bash

set -e  # Exit immediately if a command exits with a non-zero status

# Assemble the boot sector
nasm boot/bootSector.asm -f bin -o bootloader.bin

# Compile the kernel in 32-bit mode, freestanding
gcc -m32 -c kernel/kernel.c -o kernel.o -ffreestanding -fno-pie

# Link the kernel using the linker script
ld -m elf_i386 -T linker.ld -o kernel.bin kernel.o --oformat binary

# Get sizes
bootloader_size=$(stat -c %s bootloader.bin)
kernel_size=$(stat -c %s kernel.bin)
total_sectors=$(( (bootloader_size + kernel_size + 511) / 512 ))
kernel_sectors=$(( (kernel_size + 511) / 512 ))

# Pad the kernel to sector boundary if needed
padding_size=$((512 - (kernel_size % 512)))
if [ $padding_size -ne 512 ]; then
    dd if=/dev/zero bs=1 count=$padding_size >> kernel.bin
fi

# Combine bootloader and kernel into one image
cat bootloader.bin kernel.bin > os-image.bin

echo "Bootloader size: $bootloader_size bytes ($(( (bootloader_size + 511) / 512 )) sectors)"
echo "Kernel size: $kernel_size bytes ($kernel_sectors sectors)"
echo "Total image size: $((bootloader_size + kernel_size)) bytes ($total_sectors sectors)"

# Run the OS in QEMU
qemu-system-i386 -drive file=os-image.bin,format=raw