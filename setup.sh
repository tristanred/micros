#!/bin/bash

# Install the dependencies
sudo apt-get update
sudo apt-get install -y build-essential git nasm qemu hexcurse

# Download the cross compiler binaries
scp osdev@lerouxtechnologies.com:crosscompiler.tar.gz .

tar -xzf crosscompiler.tar.gz -C ~/

# Initialise a blank disk
make resetdisk