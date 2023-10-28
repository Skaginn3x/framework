#! /usr/bin/env bash
set -ex
source shared.sh


hide_output apt update
hide_output apt install -y --no-install-recommends \
  curl \
  ca-certificates \
  make \
  xz-utils \
  gcc \
  g++ \
  git \
  zip \
  unzip
