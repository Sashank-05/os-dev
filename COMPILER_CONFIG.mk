# Compiler Configuration for OS-Dev Project
# 
# Choose which compiler to use:
# - For system GCC (default):           make
# - For x86_64-elf cross-compiler:      make -f GNUmakefile CROSS=1
# - With custom compiler:               make KCC=x86_64-elf-gcc KLD=x86_64-elf-ld

# Detect if cross-compiler is available
CROSS ?= 0

ifeq ($(CROSS),1)
    # Use x86_64-elf cross-compiler
    override KCC := x86_64-elf-gcc
    override KLD := x86_64-elf-ld
    override OBJCOPY := x86_64-elf-objcopy
    $(info Using x86_64-elf cross-compiler)
else ifeq ($(shell which x86_64-elf-gcc 2>/dev/null),)
    # x86_64-elf-gcc not found, use system compiler
    $(info Using system C compiler: $(shell which cc))
else
    # x86_64-elf-gcc is available, offer to use it
    $(info Note: x86_64-elf-gcc found. Use 'make CROSS=1' to use cross-compiler)
endif

# Recommended usage:
# 
# 1. Install cross-compiler (if not already installed):
#    sudo apt install gcc-x86-64-linux-gnu binutils     # Ubuntu/Debian
#    sudo dnf install gcc binutils                      # Fedora
#    brew install x86_64-elf-gcc                         # macOS
#
# 2. Build with cross-compiler:
#    make CROSS=1
#
# 3. Or build with custom compiler:
#    make KCC=x86_64-elf-gcc KLD=x86_64-elf-ld
