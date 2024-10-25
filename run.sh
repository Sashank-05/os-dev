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

#make clean
#rm -rf iso_root 
#rm -rf obj

qemu-system-x86_64 -cdrom image.iso -m 4G -smp 2 \
-display sdl,gl=on -vga std \
-monitor stdio \
-drive if=pflash,format=raw,readonly=on,file=/usr/share/OVMF/OVMF_CODE.fd \
-drive if=pflash,format=raw,file=/usr/share/OVMF/OVMF_VARS_4M.fd \
-boot d \
-rtc base=localtime,clock=host \
