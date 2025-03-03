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
qemu-system-x86_64 \
    -cdrom image.iso \
    -m 8G \
    -no-reboot \
    -no-shutdown \
    -cpu qemu64,+la57 \
    -machine pc,acpi=on \
    -serial stdio \
    -smp 4 \
    -rtc base=localtime,clock=host \
    -vga std \
    -device VGA,vgamem_mb=128 \
    -display gtk,zoom-to-fit=off \
    -drive if=pflash,format=raw,unit=0,file=/usr/share/OVMF/OVMF_CODE.fd \
    -drive if=pflash,format=raw,unit=1,file=/usr/share/OVMF/OVMF_VARS.fd
    #-D qemu.log
    #   -d int,cpu_reset \
#-drive file=fat:rw:\hdd,id=fat32,format=raw,if=none \



