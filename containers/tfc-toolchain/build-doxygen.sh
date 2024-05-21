#!/usr/bin/env bash

set -ex

source shared.sh

VERSION=1.10.0

curl https://www.doxygen.nl/files/doxygen-1.10.0.src.tar.gz | tar xfz -

echo "Building Doxygen"

cd doxygen-1.10.0
hide_output mkdir build
hide_output cd build
hide_output cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/cpproot ..
hide_output cmake --build .
hide_output cmake --install .

cd ..
rm -rf doxygen-$VERSION

