FROM ubuntu:latest

RUN apt-get update && apt-get install sudo git -y

RUN git clone https://github.com/kratosaurion7/micros

CMD ["bash", "micros/setup.sh"]
