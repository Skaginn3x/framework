#! /usr/bin/env bash
set -ex
source shared.sh


hide_output apt update
hide_output apt install -y \
  curl \
  ca-certificates \
  make \
  xz-utils \
  gcc \
  g++ \
  git \
  zip \
  unzip \
  software-properties-common \
  python3 \
  pkg-config

# Install documentation specific packages
hide_output apt install -y \
  doxygen \
  python3-venv \
  python3-jinja2 \
  graphviz
