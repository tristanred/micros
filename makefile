### COMPILERS ###
AS=i686-elf-as
CC=i686-elf-gcc

### FLAGS ###
ASFLAGS=-g
CFLAGS=-std=gnu99 -ffreestanding -g -Og -Wall -Wextra
LFLAGS=-ffreestanding -Og -nostdlib -lgcc

### DIRECTORIES ###
BUILDDIR=build/
OUTDIR=out/

all: build

prepare:
	mkdir -p $(BUILDDIR)
	mkdir -p $(OUTDIR)

build: prepare bootloader kernel linker verify

bootloader: prepare boot.s
	$(AS) boot.s -o $(BUILDDIR)boot.o $(ASFLAGS)

kernel: prepare kernel.c
	$(CC) -c kernel.c -o $(BUILDDIR)kernel.o $(CFLAGS)

linker: prepare kernel bootloader linker.ld
	$(CC) -T linker.ld -o $(OUTDIR)myos.bin $(BUILDDIR)boot.o $(BUILDDIR)kernel.o $(LFLAGS)

verify: $(OUTDIR)myos.bin
	grub-file --is-x86-multiboot $(OUTDIR)myos.bin

.PHONY: run
run: build
	qemu-system-i386 -kernel $(OUTDIR)myos.bin -curses

debug: build
	qemu-system-i386 -s -S -kernel $(OUTDIR)myos.bin -curses

.PHONY: clean
clean:
	rm -rf $(OUTDIR) $(BUILDDIR) myos.iso isodir/

makeiso: build
	./makeiso.sh
