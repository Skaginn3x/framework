FROM debian:10-slim AS base

FROM base AS new-gcc

ENV DEBIAN_FRONTEND noninteractive

RUN mkdir /cpproot
WORKDIR /tmp
COPY ./shared.sh /tmp/

COPY install-common.sh /tmp/
RUN ./install-common.sh

COPY build-binutils.sh /tmp/
RUN ./build-binutils.sh

COPY build-gcc-from-commit.sh /tmp/
RUN ./build-gcc-from-commit.sh eb83605be3db9e8246c73755eafcac5df32ddc69

FROM base AS common

ENV DEBIAN_FRONTEND noninteractive

RUN mkdir /cpproot

COPY ./cpproot.conf /etc/ld.so.conf.d/
RUN ldconfig

RUN apt install -y --no-install-recommends

