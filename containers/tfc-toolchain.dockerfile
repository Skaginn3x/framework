FROM debian:10

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update
RUN apt-get update && apt-get install -y \
    software-properties-common  \
    git  \
    curl  \
    zip  \
    unzip  \
    tar  \
    wget  \
    lsb-release  \
    gnupg  \
    cppcheck  \
    cmake  \
    ninja-build  \
    autoconf  \
    autoconf-archive  \
    automake  \
    meson  \
    gperf  \
    libcap-dev  \
    pkg-config  \
    libtool  \
    rpm  \
    autopoint  \
    doxygen  \
    python3-venv  \
    python3-jinja2  \
    graphviz  \
    npm \
    libgmp-dev \
    libmpfr-dev \
    libmpc-dev \
    zlib1g-dev



#RUN ln -s /usr/bin/aarch64-linux-gnu-gcc-13 /usr/bin/aarch64-linux-gnu-gcc
#RUN ln -s /usr/bin/aarch64-linux-gnu-g++-13 /usr/bin/aarch64-linux-gnu-g++


WORKDIR /home/tfc/

RUN curl https://apt.llvm.org/llvm.sh -o llvm.sh && chmod +x llvm.sh && ./llvm.sh 17 all
COPY build-gcc-13.sh build-gcc-13.sh
RUN bash build-gcc-13.sh

RUN rm -rf /var/lib/apt/lists/*

RUN sed -i 's|deny own=|allow own=|g' /usr/share/dbus-1/system.conf
RUN sed -i 's|deny send_type="method_call"|allow send_type="method_call"|g' /usr/share/dbus-1/system.conf

RUN git clone https://github.com/microsoft/vcpkg /opt/vcpkg
