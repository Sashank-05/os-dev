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

# Run the ISO image in QEMU with improved graphics
# Optimized for Wayland/Hyprland

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
    -smp 4 \
    -rtc base=localtime,clock=host \
    -serial stdio \
    -vga virtio \
    -display wayland 2>/dev/null || \
qemu-system-x86_64 \
    -cdrom image.iso \
    -m 8G \
    -no-reboot \
    -no-shutdown \
    -cpu qemu64,+la57 \
    -machine pc,acpi=on \
    -smp 4 \
    -rtc base=localtime,clock=host \
    -serial stdio \
    -vga virtio \
    -display gtk
    #-D qemu.log
    #   -d int,cpu_reset \
#-drive file=fat:rw:\hdd,id=fat32,format=raw,if=none \



