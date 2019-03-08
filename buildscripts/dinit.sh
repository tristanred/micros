#!/bin/bash

# MICROS Docker Init
#
# This script should be run first to initialize the resources needed by the
# OS. Right now this is only used to create a blank disk by calling the
# make target `resetdisk`

cd ..
docker run --rm -it --mount type=bind,source="$(pwd)",target=/builder tristandube/microbuilder make resetdisk