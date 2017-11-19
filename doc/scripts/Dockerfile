FROM ubuntu:latest

WORKDIR /osdev

ADD . /osdev

RUN apt-get update && apt-get install sudo curl build-essential -y

# Run app.py when the container launches
CMD ["bash", "build-arm-compiler_unattended.sh"]
