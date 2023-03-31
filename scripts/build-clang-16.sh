#!/usr/bin/env bash

set -ex

VERSION=16.0.0

curl -L https://github.com/llvm/llvm-project/releases/download/llvmorg-$VERSION/llvm-project-$VERSION.src.tar.xz | tar xJf -

mkdir -p llvm-build
sudo mkdir -p /opt/clang-$VERSION
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;compiler-rt;lld" \
  -DLLVM_ENABLE_RUNTIMES=all \
  -DCOMPILER_RT_BUILD_SANITIZERS=ON \
  -DCOMPILER_RT_BUILD_XRAY=ON \
  -DCOMPILER_RT_BUILD_ORC=ON \
  -DCOMPILER_RT_BUILD_MEMPROF=ON \
  -DCOMPILER_RT_BUILD_PROFILE=ON \
  -DCOMPILER_RT_BUILD_GWP_ASAN=ON \
  -DCOMPILER_RT_BUILD_LIBFUZZER=ON \
  -DCOMPILER_RT_BUILD_PROFILE=ON \
  -DCMAKE_INSTALL_PREFIX=/opt/clang-$VERSION/ \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -S llvm-project-$VERSION.src/llvm -B llvm-build
cmake --build llvm-build
sudo cmake --install llvm-build

rm -rf llvm-project-$VERSION.src
rm -rf llvm-build

