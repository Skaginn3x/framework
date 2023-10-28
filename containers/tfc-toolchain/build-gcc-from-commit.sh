#! /usr/bin/env bash
set -ex
source shared.sh

GCC_TAG=$1

gcc --version
cc --version

# Fetch prerequisites
hide_output apt install libmpfr-dev libgmp3-dev libmpc-dev libisl-dev flex bzip2 bison -y --no-install-recommends

# There is some bug in gold under debian 10 that
# Makes it so that we cannot link gcc. Mold must be installed
# before this script is run and setup as the default linker /usr/bin/ld

curl -L https://github.com/gcc-mirror/gcc/archive/$GCC_TAG.tar.gz | tar -xz -C /tmp/
cd /tmp/gcc-$GCC_TAG
# --disable-shared, Since this is a development build of gcc we disable shared

hide_output ./configure --enable-languages=c,c++ --disable-shared --disable-multilib --prefix=/cpproot

hide_output make -j$(nproc)
hide_output make install
hide_output apt remove --purge -y texinfo bzip2

ln -sf /cpproot/bin/gcc /cpproot/bin/cc

rm -rf /tmp/gcc-$GCC_TAG
