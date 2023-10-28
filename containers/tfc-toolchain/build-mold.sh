#! /usr/bin/env bash
set -ex
source shared.sh

MOLD_VERSION=$1

curl -L https://github.com/rui314/mold/archive/refs/tags/v$MOLD_VERSION.tar.gz | tar -xz -C /tmp/
BUILD_DIR=/tmp/mold-$MOLD_VERSION/build
rm -rf $BUILD_DIR
hide_output mkdir $BUILD_DIR
hide_output cd $BUILD_DIR

hide_output cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/cpproot/ ..
hide_output cmake --build . -j $(nproc)
hide_output cmake --build . --target install

cd /tmp
rm -rf mold-$MOLD_VERSION
# Make mold the default linker
ln -sf /cpproot/bin/mold /usr/bin/ld
