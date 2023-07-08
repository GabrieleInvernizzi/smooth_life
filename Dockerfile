FROM ubuntu:latest
WORKDIR /smooth_life
COPY . /smooth_life/
RUN apt update && apt install -y build-essential libncurses5-dev \
    && make clean && make
ENTRYPOINT [ "./bin/smooth_life" ]