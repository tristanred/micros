### COMPILERS ###
# Note: Trying to use NASM instead of my custom gnu Assembler
AS=nasm
CC=i686-elf-gcc

### FLAGS ###
ASFLAGS=-g -felf32
CFLAGS=-std=gnu99 -ffreestanding -g -Og -O0 -Wall -Wextra
LFLAGS=-ffreestanding -Og -nostdlib -lgcc

### DIRECTORIES ###
BUILDDIR=build/
OUTDIR=out/

all: build

prepare:
	mkdir -p $(OUTDIR)
	mkdir -p $(BUILDDIR)

build: prepare bootloader kernel linker verify

bootloader: prepare boot.s
	$(AS) boot.s -o $(BUILDDIR)boot.o $(ASFLAGS)

kernel: prepare gdt.s
	$(AS) io_func.s -o $(BUILDDIR)io_funcs.o $(ASFLAGS)
	$(AS) gdt.s -o $(BUILDDIR)gdt.o $(ASFLAGS)
	$(CC) -c *.c $(CFLAGS)
	mv *.o $(BUILDDIR)

linker: prepare kernel bootloader linker.ld
	$(CC) -T linker.ld -o $(OUTDIR)myos.bin $(BUILDDIR)*.o $(LFLAGS)

verify: $(OUTDIR)myos.bin
	grub-file --is-x86-multiboot $(OUTDIR)myos.bin

.PHONY: run
run: build
	qemu-system-i386 -serial file:$(OUTDIR)serial.log -kernel $(OUTDIR)myos.bin -curses

debug: build
	qemu-system-i386 -serial file:$(OUTDIR)serial.log -s -S -kernel $(OUTDIR)myos.bin -curses

.PHONY: clean
clean:
	rm -rf $(OUTDIR) $(BUILDDIR) myos.iso isodir/

makeiso: build
	./makeiso.sh
