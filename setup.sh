#!/bin/bash

# Install the dependencies
sudo apt-get update
sudo apt-get install -y build-essential git nasm qemu grub hexcurse

# Download the cross compiler binaries
curl http://transfer.lerouxtechnologies.com/crosscompiler.tar.gz > crosscompiler.tar.gz

tar -xzf crosscompiler.tar.gz -C ~/

# Initialise a blank disk
make resetdisk