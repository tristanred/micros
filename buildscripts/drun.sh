#!/bin/bash
cd ..
docker run --rm -it --mount type=bind,source="$(pwd)",target=/builder tristandube/microbuilder make run