#!/usr/bin/env bash

set -ex

VERSION=$1

curl -L https://github.com/llvm/llvm-project/releases/download/llvmorg-$VERSION/llvm-project-$VERSION.src.tar.xz | tar xJf -

mkdir llvm-build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
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
  -DCMAKE_INSTALL_PREFIX=/cpproot \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -S llvm-project-$VERSION.src/llvm -B llvm-build
cmake --build llvm-build
cmake --install llvm-build

rm -rf llvm-project-$VERSION.src
rm -rf llvm-build

