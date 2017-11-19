#!/bin/bash

mkdir arm-cross
cd arm-cross

# Download binutils and GCC source
curl ftp://ftp.gnu.org/gnu/binutils/binutils-2.29.tar.gz > binutils-2.29.tar.gz
curl ftp://ftp.gnu.org/gnu/gcc/gcc-7.2.0/gcc-7.2.0.tar.gz > gcc-7.2.0.tar.gz

# Get dependencies
sudo apt-get update
sudo apt-get install -y libgmp3-dev libmpfr-dev libisl-dev libcloog-isl-dev libmpc-dev texinfo

# Setup variables. Compiler will be installed in the $PREFIX location
#export PREFIX="$HOME/opt/arm-cross"
export PREFIX="/osdev/output"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

# eXtract Ze Files (-xzf)
tar -xzf binutils-2.29.tar.gz
tar -xzf gcc-7.2.0.tar.gz

# Start by building binutils
mkdir build-binutils
cd build-binutils

../binutils-2.29/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

#read -r -p "Binutils build done. Install to $PREFIX ? [Yn]" response
#
#if [[ "$response" =~ ^([yY][eE][sS]|[yY])+$ ]]
#then
#    make install
#    echo "Installed binutils to $PREFIX"
#else
#    echo "Run make install manually to install the binaries."
#fi

cd ..

# Build GCC
mkdir build-gcc-7.2.0
cd build-gcc-7.2.0

# The $PREFIX/bin dir _must_ be in the PATH. We did that above.
which -- $TARGET-as || echo $TARGET-as is not in the PATH

../gcc-7.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc

make install-gcc
make install-target-libgcc

#read -r -p "GCC build done. Install to $PREFIX ? [Yn]" response
#
#if [[ "$response" =~ ^([yY][eE][sS]|[yY])+$ ]]
#then
#    make install-gcc
#    make install-target-libgcc
#    
#    echo "Installed gcc and libgcc to $PREFIX"
#else
#    echo "Run make install-gcc and make install-target-libgcc manually to install the binaries."
#fi