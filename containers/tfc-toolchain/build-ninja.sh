#! /usr/bin/env bash
set -ex
source shared.sh

NINJA_VERSION=$1

hide_output apt install -y --no-install-recommends re2c
curl -L https://github.com/ninja-build/ninja/archive/refs/tags/v$NINJA_VERSION.tar.gz | tar -xz -C /tmp/
hide_output cd /tmp/ninja-$NINJA_VERSION
hide_output cmake -Bbuild-cmake -H. -DCMAKE_INSTALL_PREFIX=/cpproot/
hide_output cmake --build build-cmake
hide_output cmake --install build-cmake
cd ..
rm -rf ninja-$NINJA_VERSION
hide_output apt remove --purge -y re2c