#!/usr/bin/env bash

set -ex
source shared.sh

LLVM_VERSION=$1

hide_output apt install -y --no-install-recommends python3
cd /tmp/
curl -L https://github.com/llvm/llvm-project/releases/download/llvmorg-$LLVM_VERSION/llvm-project-$LLVM_VERSION.src.tar.xz | tar xJf -
mkdir -p llvm-build
hide_output cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;compiler-rt;lld" \
  -DLLVM_ENABLE_RUNTIMES=all \
  -DCOMPILER_RT_BUILD_SANITIZERS=OFF \
  -DCOMPILER_RT_BUILD_XRAY=OFF \
  -DCOMPILER_RT_BUILD_ORC=OFF \
  -DCOMPILER_RT_BUILD_MEMPROF=OFF \
  -DCOMPILER_RT_BUILD_PROFILE=OFF \
  -DCOMPILER_RT_BUILD_GWP_ASAN=OFF \
  -DCOMPILER_RT_BUILD_LIBFUZZER=OFF \
  -DCOMPILER_RT_BUILD_PROFILE=OFF \
  -DLLVM_BUILD_LLVM_DYLIB=ON \
  -DLLVM_LINK_LLVM_DYLIB=ON \
  -DCMAKE_INSTALL_PREFIX=/cpproot/ \
  -S llvm-project-$LLVM_VERSION.src/llvm -B llvm-build
hide_output cmake --build llvm-build
hide_output cmake --install llvm-build

hide_output apt remove -y --purge python3

rm -rf llvm-project-$LLVM_VERSION.src
rm -rf llvm-build
