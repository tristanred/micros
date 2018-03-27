# INTRO #

MICROS is a hobby OS that I started in my spare time to explore OS concepts like
threading, virtual memory, scheduling, etc.

It is never planned to do a "usable" version. Its just for fun and maybe help
other people who want to explore the domain of OS programming.

The principal aim of this project is to be a learning experience and a bit of a 
showcase. I want to be able to make this into a clean, maintainable and 
explainable projet. I want to be able to explain and teach the various parts of 
the OS without having to go into ultra-complex-refactored-to-the-max systems.

Current features

0. Framebuffer writing
1. Serial port reading/writing using I/O routines
2. Virtual Memory using Page Tables and a heap memory manager.
3. Logging strings to serial ports. Serial port is redirected to a file on the host system.
4. Processor GDT segments for Kernel and User code/data segments.
5. Processor interrupts. Can be launched from software, currently not interrupted.
6. Disk driver for ATA disks.
7. EZ FS, a simple filesystem designed by me. Temporary crappy filesystem but works alright.
8. PCI bus device scanning. Can output device info to the serial file.
9. Error management through "kernel panic" and reporting messages on the serial port.

Features are getting refactored constantly and adapted as the architecture grows. 

# Building #

Building the OS is rather simple but needs a few specific parts. The build 
process is tested on Ubuntu 17.10 but probably works on any Unix system that 
supports GCC, QEMU and grub. The compiler needed to build the OS is a custom GCC
build meant to exclude the standard library and runtime compiler 
functionalities.

The setup can be triggered by launching [setup.sh](https://github.com/tristanred/micros/blob/master/setup.sh), check the script to
make sure you understand how it works. Basically it will download the following
packages : build-essential git nasm qemu grub hexcurse. Then it will download
a built version of the compilers, the compilers were built on Ubuntu 17.10 64bit
and probably won't work unless you are on a similar system. 
See compiler_building.md for instructions on how to create your own cross-compiler for your system. The setup scripts works unattended so don't 
start it unless you want to install a few packages and extract a few big files.

The compiler package weighs about 250MB. It gets extracted to "~/opt/cross", 
this directory is hardcoded in the makefile so make sure to update that if you 
install the compiler in an alternate path.

# Trying it out #

Once you have the compiler extracted to the correct location (~/opt/cross) you
can "cd" to the project directory and just type "make". 

The default make target will build a debug build. Once the project is built you 
can run "make debug" this will invoke gdb to launch the kernel and start 
debugging it. The debugger is configured to break upon hitting a call to the 
"Debugger()" function. So you can add calls to Debugger() at interesting places 
to step through the kernel.

# Contributing #

I'm not currently accepting contributions to the kernel, everyone is welcome to 
send in push requests to open up a discussion about their work though. 
This is simply because I want to have knowledge about every part of the system. 
I want to maintain the coding standards and make sure the documentation matches 
the code.
