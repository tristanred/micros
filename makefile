AS=i686-elf-as

CC=i686-elf-gcc
CFLAGS=-std=gnu99 -ffreestanding -O2 -Wall -Wextra

LFLAGS=-ffreestanding -O2 -nostdlib -lgcc

all: build

build: bootloader kernel linker verify

bootloader: boot.s
	$(AS) boot.s -o boot.o

kernel: kernel.c
	$(CC) -c kernel.c -o kernel.o $(CFLAGS)

linker: kernel bootloader linker.ld
	$(CC) -T linker.ld -o myos.bin boot.o kernel.o $(LFLAGS)

verify: myos.bin
	grub-file --is-x86-multiboot myos.bin

.PHONY: run
run: build
	qemu-system-i386 -kernel myos.bin -curses

.PHONY: clean
clean:
	rm -rf boot.o kernel.o myos.bin myos.iso isodir/

makeiso: build
	./makeiso.sh
