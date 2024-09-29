# Set the C compiler and debugger
CC = gcc
GDB = gdb

LD = ld
ASM = nasm

INC = -Iinc/
SRC = $(shell find . -type f -name "*.c")
ASM_SRC = $(shell find . -type f -name "*.asm")

# CRITICAL: assembly must be linked first
ORDERED_OBJ = entry.o $(src/cpu/interrupt.asm:.asm=.o)
OBJ = $(ORDERED_OBJ) $(ASM_SRC:.asm=.o) $(SRC:.c=.o)

# Compiler and linker flags
CCFLAGS = -ggdb -m32 -c -ffreestanding -fno-pie -fno-stack-protector
LDFLAGS = -m elf_i386 -T kernel.ld
LDFLAGS_BIN = ${LDFLAGS} --oformat binary
ASFLAGS = -f elf32

all: kernel kernel.elf

kernel: ${OBJ}
	${LD} -o $@ ${LDFLAGS_BIN} $^

kernel.elf: ${OBJ}
	${LD} -o $@ ${LDFLAGS} $^

%.o: %.c
	${CC} ${CCFLAGS} ${INC} -o $@ $^

%.o: %.asm
	${ASM} $< ${ASFLAGS} -o $@

clean:
	rm -f kernel kernel.elf *.o **/*.o **/**/*.o
