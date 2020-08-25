FROM ubuntu:18.04 AS build

ENV APT_KEY_DONT_WARN_ON_DANGEROUS_USAGE=DontWarn

RUN mkdir /chat
COPY CMakeLists.txt /chat
COPY main.cpp /chat

WORKDIR /chat

RUN apt-get update && apt-get install -y \
    software-properties-common \
    build-essential \
    ca-certificates \
    wget

RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | apt-key add -
RUN apt-add-repository "deb https://apt.kitware.com/ubuntu/ bionic main"
RUN apt-get update && apt-get install -y cmake

RUN wget https://dl.bintray.com/boostorg/release/1.74.0/source/boost_1_74_0.tar.bz2
RUN tar xvf boost_1_74_0.tar.bz2
ENV BOOST_ROOT=/chat/boost_1_74_0
WORKDIR $BOOST_ROOT
RUN ./bootstrap.sh --with-libraries=program_options
RUN ./b2

WORKDIR /chat/build
RUN cmake ..
RUN make

FROM ubuntu:18.04 AS runtime
WORKDIR /chat
ENV BOOST_LIBS=/chat/boost_1_74_0/stage/lib
COPY --from=build /chat/build/server ./
COPY --from=build $BOOST_LIBS/libboost_program_options.so ./
COPY --from=build $BOOST_LIBS/libboost_program_options.so.1.74.0 ./
ENV LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./

ENTRYPOINT ["/chat/server"]
CMD ["--port", "8080"]
