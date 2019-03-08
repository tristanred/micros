#!/bin/bash

# MICROS Docker Clean
#
# Runs the clean target. Basically removes the out/ directory

cd ..
docker run --rm -it --mount type=bind,source="$(pwd)",target=/builder tristandube/microbuilder make clean