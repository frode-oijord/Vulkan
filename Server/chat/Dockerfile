FROM alpine:3.12 AS build

RUN mkdir /chat
COPY CMakeLists.txt /chat
COPY main.cpp /chat

WORKDIR /chat

RUN apk update && apk add --no-cache \
    build-base \
    linux-headers \
    boost-dev \
    cmake

WORKDIR /chat/build
RUN cmake ..
RUN make

FROM alpine:3.12 AS runtime

RUN apk update && apk add --no-cache \
    libstdc++

WORKDIR /chat
COPY --from=build /chat/build/server ./
COPY --from=build /usr/lib/libboost_program_options.so.1.72.0 ./lib/
ENV LD_LIBRARY_PATH=./lib
CMD ["/chat/server", "--port", "8080"]
