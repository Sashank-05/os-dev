#!/bin/bash

# Run make to build the project
make

# Check if make was successful
if [ $? -ne 0 ]; then
    echo "Make failed. Exiting."
    exit 1
fi

# Create an ISO image
./mkiso.sh 

# Check if mkisofs was successful
if [ $? -ne 0 ]; then
    echo "mkiso failed. Exiting."
    exit 1
fi

# Run the ISO image in QEMU with 1920x1080 resolution

qemu-system-x86_64 -cdrom image.iso -m 2048 -smp 2 \
-display sdl,gl=on -vga std \
-vga std -monitor stdio -m 4G \
-bios /usr/share/OVMF/OVMF_CODE.fd \
-boot d
