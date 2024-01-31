#!/usr/bin/env bash

set -ex

source shared.sh

VERSION=$1

hide_output apt install bzip2 bison file texinfo -y --no-install-recommends

curl https://ftp.gnu.org/gnu/binutils/binutils-$VERSION.tar.bz2 | tar xfj -

mkdir binutils-build
cd binutils-build
hide_output ../binutils-$VERSION/configure --prefix=/cpproot
hide_output make -j$(nproc)
hide_output make install

cd ..
rm -rf binutils-build
rm -rf binutils-$VERSION
