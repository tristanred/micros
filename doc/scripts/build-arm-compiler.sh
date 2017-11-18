#!/bin/bash

mkdir arm-cross
cd arm-cross

curl ftp://ftp.gnu.org/gnu/binutils/binutils-2.9.1.tar.gz > binutils-2.9.1.tar.gz

curl ftp://ftp.gnu.org/gnu/gcc/gcc-7.2.0/gcc-7.2.0.tar.gz > gcc-7.2.0.tar.gz

sudo apt-get update
sudo apt-get install libgmp3-dev libmpfr-dev libisl-dev libcloog-isl-dev libmpc-dev texinfo

export PREFIX="$HOME/opt/arm-cross"
export TARGET=arm-none-eabi
export PATH="$PREFIX/bin:$PATH"

tar -xzf binutils-2.9.1.tar.gz
tar -xzf gcc-7.2.0.tar.gz

mkdir build-binutils
cd build-binutils

../binutils-2.9.1/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

cd ..

mkdir build-gcc-7.2.0
cd build-gcc-7.2.0

# The $PREFIX/bin dir _must_ be in the PATH. We did that above.
which -- $TARGET-as || echo $TARGET-as is not in the PATH

../gcc-7.2.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
make all-gcc
make all-target-libgcc
make install-gcc
make install-target-libgcc