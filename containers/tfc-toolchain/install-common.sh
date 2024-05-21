#! /usr/bin/env bash
set -ex
source shared.sh


hide_output apt update
hide_output apt install -y --no-install-recommends \
  curl \
  ca-certificates \
  make \
  automake \
  xz-utils \
  git \
  zip \
  unzip \
  software-properties-common \
  pkg-config \
  autoconf \
  autoconf-archive \
  libtool \
  autopoint \
  gperf \
  python3-jinja2 \
  python3-venv \
  gcc-14 \
  g++-14 \
  g++-14-aarch64-linux-gnu \
  libltdl-dev # Required for autoconf

# Install documentation specific packages
hide_output apt install -y --no-install-recommends \
  python3-venv \
  python3-jinja2 \
  graphviz

# Packaging specific packages
hide_output apt install -y --no-install-recommends \
  rpm
