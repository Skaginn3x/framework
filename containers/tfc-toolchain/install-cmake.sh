#! /usr/bin/env bash
set -ex
source shared.sh

CMAKE_VERSION=$1
# Fetch cmake 3.28.0-rc3
curl -L https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-linux-x86_64.sh -o /tmp/cmake.sh
hide_output chmod +x /tmp/cmake.sh
hide_output /tmp/cmake.sh --skip-license --prefix=/cpproot/
hide_output rm /tmp/cmake.sh
