FROM alpine:3.12 AS build

WORKDIR /chat
COPY CMakeLists.txt ./
COPY main.cpp ./
COPY json.hpp ./

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
COPY client ./client
COPY --from=build /chat/build/server ./
COPY --from=build /usr/lib/libboost_program_options.so.1.72.0 ./lib/
ENV LD_LIBRARY_PATH=./lib
CMD ["/chat/server", "--port", "8080", "--root", "./client"]
