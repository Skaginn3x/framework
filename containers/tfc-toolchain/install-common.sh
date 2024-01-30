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
  python3-distutils \
  python3-jinja2 \
  python3-venv \
  gcc-13 \
  g++-13 \
  g++-13-aarch64-linux-gnu


# Speciality installs
hide_output apt install -y --no-install-recommends \
  libltdl-dev # This package is required for autoconf to succeed when building libxcrypt


# Install documentation specific packages
#hide_output apt install -y \
#  doxygen \
#  python3-venv \
#  python3-jinja2 \
#  graphviz
