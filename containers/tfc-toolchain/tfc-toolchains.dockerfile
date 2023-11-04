FROM debian:10-slim AS base

ENV DEBIAN_FRONTEND noninteractive

ENV PATH=/cpproot/bin:$PATH
ENV PKG_CONFIG_PATH=/cpproot/lib/pkgconfig

RUN mkdir /cpproot
WORKDIR /tmp
COPY ./shared.sh /tmp/

COPY ./cpproot.conf /etc/ld.so.conf.d/
RUN ldconfig

COPY install-common.sh /tmp/
RUN ./install-common.sh

COPY install-cmake.sh /tmp/
RUN ./install-cmake.sh 3.28.0-rc3

COPY build-ninja.sh /tmp/
RUN ./build-ninja.sh 1.11.1

ENV VCPKG_FORCE_SYSTEM_BINARIES=1

RUN cd /opt && git clone https://github.com/microsoft/vcpkg.git && cd vcpkg && ./bootstrap-vcpkg.sh -musl
RUN cd /opt && chmod 775 vcpkg

RUN apt-get update && apt-get install -y --no-install-recommends dbus
RUN mkdir /var/run/dbus/
RUN sed -i 's|deny own=|allow own=|g' /usr/share/dbus-1/system.conf
RUN sed -i 's|deny send_type="method_call"|allow send_type="method_call"|g' /usr/share/dbus-1/system.conf

FROM base AS gcc-14

COPY build-binutils.sh /tmp/
RUN ./build-binutils.sh

COPY build-gcc-from-commit.sh /tmp/
RUN ./build-gcc-from-commit.sh eb83605be3db9e8246c73755eafcac5df32ddc69

RUN apt remove -y gcc g++

COPY build-mold.sh /tmp/
RUN ./build-mold.sh 2.3.1
#RUN apt update && apt install -y --no-install-recommends libisl-dev libmpc-dev libc-dev

RUN apt autoremove -y
RUN apt clean all
RUN rm -rf /var/lib/apt/lists/*
RUN rm -rf /tmp/*

FROM base as clang-17

COPY build-clang.sh /tmp/
RUN ./build-clang.sh 17.0.3

ENV CC=clang
ENV CXX=clang++
ENV CFLAGS="-stdlib=libc++"
ENV CXXFLAGS="-stdlib=libc++"
ENV LDFLAGS="-fuse-ld=lld -stdlib=libc++ -rtlib=compiler-rt"
ENV AR="llvm-ar"

COPY build-mold.sh /tmp/
RUN ./build-mold.sh 2.3.1

RUN apt remove -y gcc g++
RUN apt update && apt install -y --no-install-recommends libc-dev
RUN apt autoremove -y
RUN apt clean all
RUN rm -rf /var/lib/apt/lists/*
RUN rm -rf /tmp/*

FROM clang-17 as packaging

# Why does nodejs need gcc, relates to atomic but atomic only is not enough why?
RUN apt update && apt install -y --no-install-recommends libatomic1 gcc

COPY build-node.sh /tmp/
COPY ./shared.sh /tmp/
RUN ./build-node.sh 21.1.0

RUN apt remove -y gcc
RUN apt autoremove -y
RUN apt clean all
