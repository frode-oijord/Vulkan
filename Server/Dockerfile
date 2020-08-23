FROM ubuntu:18.04

ENV APT_KEY_DONT_WARN_ON_DANGEROUS_USAGE=DontWarn

RUN mkdir /server
COPY CMakeLists.txt /server
COPY main.cpp /server

WORKDIR /server

RUN apt-get update && apt-get install -y \
    software-properties-common \
    build-essential \
    ca-certificates \
    wget

RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | apt-key add -
RUN apt-add-repository "deb https://apt.kitware.com/ubuntu/ bionic main"
RUN apt-get update
RUN apt-get install -y cmake

RUN wget https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.bz2
RUN tar xvf boost_1_74_0.tar.bz2
ENV BOOST_ROOT=/server/boost_1_74_0

RUN mkdir build
WORKDIR build
RUN cmake ..
RUN make

CMD ["./server"]
