FROM ubuntu:latest

RUN apt-get update && apt-get install git build-essential git nasm qemu hexcurse curl -y

RUN git clone https://github.com/kratosaurion7/micros

RUN chmod +x micros/setup.sh

WORKDIR micros

RUN /bin/bash setup.sh
