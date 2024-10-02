#!/bin/bash

# Compile the kernel
(cd kernel && make clean && make)
make_result=$?

if [ "$make_result" -eq 0 ]; then
    # Create a blank disk image (40MB)
    dd if=/dev/zero of=os.img bs=1M count=64

    # Create a partition table
    parted os.img mklabel msdos
    parted os.img mkpart primary ext2 1MiB 100%
    parted os.img set 1 boot on

    # Set up loopback device
    loop_device=$(sudo losetup -Pf --show os.img)

    # Format the partition
    sudo mkfs.ext2 "${loop_device}p1"

    # Mount the partition
    mkdir -p mnt
    sudo mount "${loop_device}p1" mnt

    # Install GRUB (explicitly for BIOS)
    sudo mkdir -p mnt/boot/grub
    sudo grub-install --target=i386-pc \
                      --boot-directory=mnt/boot \
                      --no-floppy \
                      --modules="multiboot biosdisk part_msdos ext2" \
                      "${loop_device}"

    # Copy the kernel
    sudo cp kernel/kernel.elf mnt/boot/kernel.elf

    # Create GRUB config
    cat << EOF | sudo tee mnt/boot/grub/grub.cfg
menuentry "My OS" {
    multiboot /boot/kernel.elf
    boot
}
EOF

    # Cleanup
    sudo umount mnt
    sudo losetup -d "${loop_device}"
    rmdir mnt

    echo "Build and GRUB installation completed successfully"

    # Run in QEMU (explicitly in BIOS mode)
    qemu-system-i386 -drive format=raw,file=os.img \
                     -m 4G \
                     #-machine pc \
                     #-no-acpi \
                     #-device isa-debug-exit,iobase=0xf4,iosize=0x04
else
    echo "Build failed with error code $make_result"
fi