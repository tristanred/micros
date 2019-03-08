#!/bin/bash

# MICROS Docker Debug
#
# This script is used to launch the OS in debug mode. Right now, through
# docker, it might be difficult to hook GDB into it. I have not tested this
# functionality but it might be better to avoid debugging through Docker anyway

cd ..
docker run --rm -it --mount type=bind,source="$(pwd)",target=/builder tristandube/microbuilder make debug