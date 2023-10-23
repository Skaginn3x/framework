FROM ubuntu:23.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install software-properties-common -y
RUN add-apt-repository ppa:ubuntu-toolchain-r/ppa -y
RUN apt-get update && apt-get install -y git curl zip unzip tar wget lsb-release gnupg cppcheck cmake g++-13 g++-13-aarch64-linux-gnu ninja-build autoconf autoconf-archive automake meson gperf libcap-dev pkg-config libtool rpm autopoint doxygen python3-venv python3-jinja2 graphviz npm
RUN ln -s /usr/bin/aarch64-linux-gnu-gcc-13 /usr/bin/aarch64-linux-gnu-gcc
RUN ln -s /usr/bin/aarch64-linux-gnu-g++-13 /usr/bin/aarch64-linux-gnu-g++

RUN apt-get update && apt-get install -y dbus-broker

RUN git clone https://github.com/microsoft/vcpkg /opt/vcpkg

WORKDIR /home/tfc/

RUN curl https://apt.llvm.org/llvm.sh -o llvm.sh && chmod +x llvm.sh && ./llvm.sh 17 all

RUN rm -rf /var/lib/apt/lists/*

RUN sed -i 's|deny own=|allow own=|g' /usr/share/dbus-1/system.conf
RUN sed -i 's|deny send_type="method_call"|allow send_type="method_call"|g' /usr/share/dbus-1/system.conf

#RUN addgroup tfc
#RUN adduser -S tfc -G tfc
#USER tfc
