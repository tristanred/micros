### COMPILERS ###
# Note: Trying to use NASM instead of my custom gnu Assembler
AS=nasm
CC=i686-elf-gcc

### FLAGS ###
ASFLAGS=-g -felf32
CFLAGS=-std=gnu99 -ffreestanding -g3 -Og -O0 -Wall -Wextra
LFLAGS=-ffreestanding -Og -nostdlib -lgcc

### DIRECTORIES ###
BUILDDIR=build/
OUTDIR=out/

all: build

prepare:
	mkdir -p $(OUTDIR)
	mkdir -p $(BUILDDIR)

build: prepare bootloader kernel linker verify

bootloader: prepare boot.asm
	$(AS) boot.asm -o $(BUILDDIR)boot.o $(ASFLAGS)

kernel: prepare agdt.asm
	$(AS) io_func.s -o $(BUILDDIR)io_funcs.o $(ASFLAGS)
	$(AS) agdt.asm -o $(BUILDDIR)agdt.o $(ASFLAGS)
	$(AS) aidt.asm -o $(BUILDDIR)aidt.o $(ASFLAGS)
	$(AS) interrupts.asm -o $(BUILDDIR)interrupts.o $(ASFLAGS)
	$(AS) memory.asm -o $(BUILDDIR)amemory.o $(ASFLAGS)
	$(CC) -c *.c $(CFLAGS)
	mv *.o $(BUILDDIR)

linker: prepare kernel bootloader linker.ld
	$(CC) -T linker.ld -o $(OUTDIR)myos.bin $(BUILDDIR)*.o $(LFLAGS)

verify: $(OUTDIR)myos.bin
	grub-file --is-x86-multiboot $(OUTDIR)myos.bin

# Commandline flags passed to the kernel multiboot
# d : Debug flags. Currently passed to all targets for development.
# g : enable the VGA graphics framebuffer. Text mode is used by default.
#

.PHONY: run
run: build
	qemu-system-i386 -m 128M -append "-f d" -drive format=raw,file=disk.img,if=ide -serial file:$(OUTDIR)serial.log -kernel $(OUTDIR)myos.bin -curses

runiso: makeiso
	qemu-system-i386 -m 128M -append "-f d" -drive format=raw,file=disk.img,if=ide -serial file:$(OUTDIR)serial.log $(OUTDIR)myos.iso

debugiso: makeiso
	qemu-system-i386 -m 128M -append "-f gd" -drive format=raw,file=disk.img,if=ide -serial file:$(OUTDIR)serial.log -s -S $(OUTDIR)myos.iso

debug: build
	qemu-system-i386 -m 128M -append "-f gd" -drive format=raw,file=disk.img,if=ide -serial file:$(OUTDIR)serial.log -s -S -kernel $(OUTDIR)myos.bin -curses

resetdisk:
	rm disk.img
	qemu-img create -f raw disk.img 32M

.PHONY: clean
clean:
	rm -rf $(OUTDIR) $(BUILDDIR) myos.iso isodir/

makeiso: build
	./makeiso.sh
