FROM ubuntu:23.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update && apt-get install -y git curl zip unzip tar wget lsb-release gnupg cppcheck software-properties-common cmake g++-13 g++-13-aarch64-linux-gnu ninja-build autoconf autoconf-archive meson gperf libcap-dev pkg-config libtool rpm autopoint doxygen python3-venv python3-sphinx python3-sphinx-rtd-theme python3-breathe graphviz python3-myst-parser npm
RUN apt-get update && apt-get install dbus-broker
RUN git clone https://github.com/microsoft/vcpkg /opt/vcpkg

WORKDIR /home/tfc/

RUN curl https://apt.llvm.org/llvm.sh -o llvm.sh && chmod +x llvm.sh && ./llvm.sh 16 all


#COPY commissioning.sh .
#RUN ./commissioning.sh
#


