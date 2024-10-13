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

# Run the ISO image in QEMU
qemu-system-x86_64 -cdrom image.iso