cd ..
docker run --rm -it --mount type=bind,source="$(pwd)",target=/builder microbuilder make clean