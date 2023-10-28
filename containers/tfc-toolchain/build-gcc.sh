#!/usr/bin/env bash
set -ex

source shared.sh

GCC_VERSION=$1

cd /tmp
curl https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz | tar xJf -
cd gcc-$GCC_VERSION

hide_output apt install -y --no-install-recommends lbzip2 bzip2
./contrib/download_prerequisites
mkdir ../gcc-build
cd ../gcc-build
hide_output ../gcc-$GCC_VERSION/configure \
    --prefix=/usr \
    --enable-languages=c,c++ \
	--disable-multilib \
    --disable-gnu-unique-object
hide_output make -j32
hide_output make install

echo "/usr/lib64/" > /etc/ld.so.conf.d/custom.conf
ldconfig

cd ..
rm -rf gcc-build
rm -rf gcc-$GCC_VERSION
