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
  unzip \
  software-properties-common \
  pkg-config \
  autoconf

# Install documentation specific packages
#hide_output apt install -y \
#  doxygen \
#  python3-venv \
#  python3-jinja2 \
#  graphviz
