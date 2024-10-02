qemu-system-i386 \
    -drive format=raw,file=os.img \
    -m 4G \
    -d int,cpu_reset \
    -no-reboot \
    -monitor stdio \
    -machine pc \
    -no-acpi