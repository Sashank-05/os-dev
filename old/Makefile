CC = gcc
LD = ld 
ASM = nasm

INC = -Iinc/

SRC = $(shell find . -type f -name "*.c")
ASM_SRC = $(shell find . -type f -name "*.asm")

ENTRY = src/entry.asm
OBJ = $(ENTRY:.asm=.o) $(ASM_SRC:.asm=.o) $(SRC:.c=.o)

CCFLAGS = -ggdb -m64 -c -ffreestanding -fno-stack-protector
LDFLAGS = -m elf_x86_64 -T kernel.ld
LDFLAGS_BIN = ${LDFLAGS} --oformat binary
ASFLAGS = -f elf64

all: kernel.elf iso run

kernel.elf: $(OBJ)
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CCFLAGS) $(INC) -o $@ $<

%.o: %.asm
	$(ASM) $(ASFLAGS) -o $@ $<

iso:
	mkdir -p isodir/boot/grub
	cp kernel.elf isodir/boot/kernel.elf
	cp grub.cfg isodir/boot/grub/grub.cfg
	grub-mkrescue -o kernel.iso isodir
	rm -rf isodir
	rm -f kernel.elf $(OBJ)


run:
	qemu-system-x86_64 -cdrom kernel.iso -m 4g -d int -no-reboot

clean:
	rm -f $(OBJ) kernel.elf

 
