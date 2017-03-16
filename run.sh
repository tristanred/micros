#!/bin/bash

#qemu-system-i386 -cdrom myos.iso -curses

qemu-system-i386 -kernel out/myos.bin -curses
