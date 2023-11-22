FROM ubuntu:23.10

ENV DEBIAN_FRONTEND noninteractive

ENV PATH=/cpproot/bin:$PATH
ENV LD_LIBRARY_PATH=/cpproot/lib64:/cpproot/lib32:/cpproot/lib
ENV PKG_CONFIG_PATH=/cpproot/lib/pkgconfig

# Store custom artifacts in /cpproot
RUN mkdir -p /cpproot/bin
WORKDIR /tmp
COPY ./shared.sh /tmp/

# Required dependencies for many steps
COPY install-common.sh /tmp/
RUN ./install-common.sh

# Give gcc-13 some better names
RUN ln -sf /usr/bin/gcc-13 /cpproot/bin/gcc
RUN ln -sf /usr/bin/g++-13 /cpproot/bin/g++
RUN ln -sf /usr/bin/gcc-13 /cpproot/bin/cc
RUN ln -sf /usr/bin/aarch64-linux-gnu-gcc-13 /cpproot/bin/aarch64-linux-gnu-gcc
RUN ln -sf /usr/bin/aarch64-linux-gnu-g++-13 /cpproot/bin/aarch64-linux-gnu-g++

# Cmake is required for mold
# Install cmake
COPY install-cmake.sh /tmp/
RUN ./install-cmake.sh 3.27.0

# Install ninja
COPY build-ninja.sh /tmp/
RUN ./build-ninja.sh 1.11.1

# Install llvm
COPY build-clang.sh /tmp/
RUN ./build-clang.sh 17.0.3

# Install gcc development
# COPY build-gcc-from-commit.sh /tmp/
# RUN ./build-gcc-from-commit.sh 39a11d8e0b9cc3ac5d7d1bfaef75639fc557fbe0

# Install mold
COPY build-mold.sh /tmp/
RUN ./build-mold.sh 2.3.1

# Install node
COPY build-node.sh /tmp/
RUN ./build-node.sh 21.1.0


# Suppress download of CMake and Ninja and use the ones we built
ENV VCPKG_FORCE_SYSTEM_BINARIES=1

RUN cd /opt && git clone https://github.com/microsoft/vcpkg.git && cd vcpkg && ./bootstrap-vcpkg.sh -musl
RUN cd /opt && chmod 775 vcpkg

# Setup dbus for testing
RUN apt-get update && apt-get install -y --no-install-recommends dbus
RUN mkdir -p /var/run/dbus/
RUN sed -i 's|deny own=|allow own=|g' /usr/share/dbus-1/system.conf
RUN sed -i 's|deny send_type="method_call"|allow send_type="method_call"|g' /usr/share/dbus-1/system.conf

RUN apt autoremove -y
RUN apt clean all
RUN rm -rf /var/lib/apt/lists/*
RUN rm -rf /tmp/*
