# boost-build requires newer compiler than gcc 8
FROM ubuntu:23.04 AS base

ENV DEBIAN_FRONTEND noninteractive

ENV PATH=/cpproot/bin:$PATH
ENV PKG_CONFIG_PATH=/cpproot/lib/pkgconfig
ENV LD_LIBRARY_PATH=/cpproot/lib:/cpproot/lib/x86_64-unknown-linux-gnu:$LD_LIBRARY_PATH
ENV LIBRARY_PATH=/cpproot/lib:/cpproot/lib/x86_64-unknown-linux-gnu:$LIBRARY_PATH

RUN mkdir -p /cpproot/bin
RUN mkdir /cpproot/include
RUN mkdir /cpproot/lib
WORKDIR /tmp
COPY ./shared.sh /tmp/

COPY ./cpproot.conf /etc/ld.so.conf.d/
RUN ldconfig

COPY install-common.sh /tmp/
RUN ./install-common.sh

# Give gcc-13 some better names
RUN ln -sf /usr/bin/gcc-13 /cpproot/bin/gcc
RUN ln -sf /usr/bin/g++-13 /cpproot/bin/g++
RUN ln -sf /usr/bin/gcc-13 /cpproot/bin/cc
RUN ln -sf /usr/bin/aarch64-linux-gnu-gcc-ar-13 /cpproot/bin/aarch64-linux-gnu-ar
RUN ln -sf /usr/bin/aarch64-linux-gnu-strip /cpproot/bin/aarch64-linux-gnu-strip
RUN ln -sf /usr/bin/aarch64-linux-gnu-gcc-13 /cpproot/bin/aarch64-linux-gnu-gcc
RUN ln -sf /usr/bin/aarch64-linux-gnu-g++-13 /cpproot/bin/aarch64-linux-gnu-g++
RUN ln -sf /usr/aarch64-linux-gnu /cpproot/aarch64-linux-gnu
RUN ln -sf /usr/include/aarch64-linux-gnu /cpproot/include/aarch64-linux-gnu
RUN ln -sf /usr/lib/aarch64-linux-gnu /cpproot/lib/aarch64-linux-gnu

COPY install-cmake.sh /tmp/
RUN ./install-cmake.sh 3.28.1

COPY build-ninja.sh /tmp/
RUN ./build-ninja.sh 1.11.1

# This has to happen after gcc-13 and cmake
COPY build-mold.sh /tmp/
RUN ./build-mold.sh 2.4.0

ENV VCPKG_FORCE_SYSTEM_BINARIES=1

RUN cd /opt && git clone https://github.com/microsoft/vcpkg.git && cd vcpkg && ./bootstrap-vcpkg.sh -musl
RUN cd /opt && chmod 775 vcpkg

RUN apt-get update && apt-get install -y --no-install-recommends dbus-broker
RUN mkdir -p /var/run/dbus/
RUN sed -i 's|deny own=|allow own=|g' /usr/share/dbus-1/system.conf
RUN sed -i 's|deny send_type="method_call"|allow send_type="method_call"|g' /usr/share/dbus-1/system.conf

FROM base AS gcc-13

COPY build-binutils.sh /tmp/
RUN ./build-binutils.sh 2.40

# COPY build-gcc-from-commit.sh /tmp/
# RUN ./build-gcc-from-commit.sh eb83605be3db9e8246c73755eafcac5df32ddc69
RUN apt update
# RUN apt install g++-13-aarch64-linux-gnu -y # Already done in common
RUN ln -sf /usr/bin/aarch64-linux-gnu-gcc-13 /cpproot/bin/aarch64-linux-gnu-gcc
RUN ln -sf /usr/bin/aarch64-linux-gnu-g++-13 /cpproot/bin/aarch64-linux-gnu-g++
# RUN apt remove -y gcc g++

#RUN apt update && apt install -y --no-install-recommends libisl-dev libmpc-dev libc-dev

RUN apt autoremove -y
RUN apt clean all
RUN rm -rf /var/lib/apt/lists/*
RUN rm -rf /tmp/*

FROM gcc-13 as clang-17

COPY shared.sh /tmp/
COPY build-clang.sh /tmp/
RUN ./build-clang.sh 17.0.6

#ENV CC=clang
#ENV CXX=clang++
#ENV CFLAGS="-stdlib=libc++"
#ENV CXXFLAGS="-stdlib=libc++"
#ENV LDFLAGS="-fuse-ld=lld -stdlib=libc++"
#ENV AR="llvm-ar"

# THIS is so much crap, boost-build vcpkg port strictly requires gcc, need to fix port to bypass
#RUN apt remove -y gcc g++
RUN apt update && apt install -y --no-install-recommends libc-dev
RUN apt autoremove -y
RUN apt clean all
RUN rm -rf /var/lib/apt/lists/*
RUN rm -rf /tmp/*

FROM clang-17 as packaging

# Why does nodejs need gcc, relates to atomic but atomic only is not enough why?
RUN apt update && apt install -y --no-install-recommends libatomic1

COPY build-node.sh /tmp/
COPY ./shared.sh /tmp/
RUN ./build-node.sh 21.1.0

# THIS is so much crap, boost-build vcpkg port strictly requires gcc, need to fix port to bypass
#RUN apt remove -y gcc
RUN apt autoremove -y
RUN apt clean all
