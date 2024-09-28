#!/bin/bash

# Assemble the boot sector
nasm boot/bootSector.asm -f bin -o bootloader.bin

# Check if the bootloader was created successfully
if [[ ! -f bootloader.bin ]]; then
    echo "Failed to assemble bootloader."
    exit 1
fi

# Compile the kernel in 32-bit mode, freestanding
gcc -m32 -c kernel/kernel.c -o kernel.o -ffreestanding -fno-pie 

# Check if the kernel object was created successfully
if [[ ! -f kernel.o ]]; then
    echo "Failed to compile kernel."
    exit 1
fi

# Link the kernel using the linker script
ld -m elf_i386 -T linker.ld -o kernel.bin kernel.o

# Check if the kernel binary was created successfully
if [[ ! -f kernel.bin ]]; then
    echo "Failed to link kernel."
    exit 1
fi

# Combine bootloader and kernel into one image
cat bootloader.bin kernel.bin > os-image.bin  # Output as .bin file

# Check if the OS image was created successfully
if [[ ! -f os-image.bin ]]; then
    echo "Failed to create OS image."
    exit 1
fi

# Get the boot load size dynamically
boot_load_size=$(stat -c %s os-image.bin)

# Create ISO file from the bootloader and kernel
#genisoimage -o os-image.iso -b os-image.bin -no-emul-boot -boot-load-size $(( (boot_load_size + 511) / 512 )) -boot-info-table os-image.bin

# Check if the ISO file was created successfully
if [[ ! -f os-image.iso ]]; then
    echo "Failed to create ISO image."
    exit 1
fi

# Run the OS in QEMU
qemu-system-i386 -drive file=os-image.bin,format=raw -boot d
