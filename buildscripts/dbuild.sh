#!/bin/bash

# MICROS Docker Build
#
# This builds the OS and outputs the results in the out/ directory.

cd ..
docker run --rm -it --mount type=bind,source="$(pwd)",target=/builder tristandube/microbuilder make build