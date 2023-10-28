#! /usr/bin/env bash
set -ex
source shared.sh

NODE_VERSION=$1


# Fetch prerequisites
hide_output apt install python3 -y --no-install-recommends

curl -L https://github.com/nodejs/node/archive/refs/tags/v$NODE_VERSION.tar.gz | tar -xz -C /tmp/
cd /tmp/node-$NODE_VERSION

hide_output ./configure --prefix=/cpproot

hide_output make -j$(nproc)
hide_output make install

rm -rf /tmp/node-$NODE_VERSION
