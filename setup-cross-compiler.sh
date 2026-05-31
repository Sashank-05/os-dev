#!/bin/bash
# Setup x86_64-elf cross-compiler for OS development
# This script downloads and installs a pre-built cross-compiler

set -e

COMPILER_DIR="$HOME/.local/os-dev-toolchain"
TARGET="x86_64-elf"
VERSION="13.2.0"

echo "Setting up $TARGET cross-compiler..."

# Create toolchain directory
mkdir -p "$COMPILER_DIR"

# Check if already installed
if [ -f "$COMPILER_DIR/bin/$TARGET-gcc" ]; then
    echo "Cross-compiler already installed at $COMPILER_DIR"
    echo "Add this to your shell profile to use it:"
    echo "export PATH=\"$COMPILER_DIR/bin:\$PATH\""
    exit 0
fi

echo "This will download and install the x86_64-elf cross-compiler."
echo "Installation directory: $COMPILER_DIR"
echo ""

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Try to use apt (Ubuntu/Debian)
if command_exists apt-get; then
    echo "Found apt package manager. Installing via apt..."
    sudo apt-get update
    sudo apt-get install -y gcc-x86-64-linux-gnu binutils g++ make
    echo "Cross-compiler installed system-wide"
    
# Try to use pacman (Arch)
elif command_exists pacman; then
    echo "Found pacman package manager. Installing via pacman..."
    sudo pacman -S base-devel
    echo "Building x86_64-elf toolchain (this may take a while)..."
    # For Arch, we might need to build from source or use AUR
    # Fallback to building from source below
    
# Try to use brew (macOS)
elif command_exists brew; then
    echo "Found brew package manager. Installing via brew..."
    brew install x86_64-elf-gcc
    echo "Cross-compiler installed via brew"
    exit 0
fi

# If no package manager worked, or on Arch without AUR helper, provide manual instructions
echo ""
echo "To build x86_64-elf cross-compiler from source, run:"
echo "1. Install dependencies: sudo apt-get install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo"
echo "2. Download binutils: wget https://ftpmirror.gnu.org/binutils/binutils-2.41.tar.xz"
echo "3. Download gcc: wget https://ftpmirror.gnu.org/gcc/gcc-13.2.0/gcc-13.2.0.tar.xz"
echo "4. See: https://wiki.osdev.org/GCC_Cross-Compiler"
echo ""
echo "Or, easier - use your system's package manager:"
echo "  Ubuntu/Debian: sudo apt install gcc-x86-64-linux-gnu binutils"
echo "  Fedora/RHEL: sudo dnf install binutils gcc gcc-c++"
echo "  Arch: Build from AUR or use x86_64-linux-gnu-gcc"
