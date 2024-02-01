#! /usr/bin/env bash
set -ex
source shared.sh

GCC_TAG=$1

# Fetch prerequisites
hide_output apt install libmpfr-dev libgmp3-dev libmpc-dev libisl-dev flex bzip2 bison -y --no-install-recommends

# There is some bug in gold under debian 10 that
# Makes it so that we cannot link gcc. Mold must be installed
# before this script is run and setup as the default linker /usr/bin/ld

curl -L https://gnu.mirror.constant.com/mpfr/mpfr-4.2.1.tar.xz | tar -xJ -C /tmp/
curl -L https://gnu.mirror.constant.com/gmp/gmp-6.3.0.tar.xz | tar -xJ -C /tmp/
curl -L https://gnu.mirror.constant.com/mpc/mpc-1.3.1.tar.gz | tar -xz -C /tmp/
wget ftp://gcc.gnu.org/pub/gcc/infrastructure/isl-0.12.2.tar.bz2
wget ftp://gcc.gnu.org/pub/gcc/infrastructure/cloog-0.18.1.tar.gz

curl -L https://github.com/gcc-mirror/gcc/archive/$GCC_TAG.tar.gz | tar -xz -C /tmp/
ln -s /tmp/mpfr-4.2.1 /tmp/gcc-$GCC_TAG/mpfr
ln -s /tmp/gmp-6.3.0 /tmp/gcc-$GCC_TAG/gmp
ln -s /tmp/mpc-1.3.1 /tmp/gcc-$GCC_TAG/mpc
cd /tmp/gcc-$GCC_TAG

hide_output ./configure --enable-languages=c,c++ --disable-multilib --prefix=/cpproot #--target=aarch64-linux --program-suffix=-aarch64-linux-gnu

hide_output make -j$(nproc)
hide_output make install
hide_output apt remove --purge -y texinfo bzip2

ln -sf /cpproot/bin/gcc /cpproot/bin/cc

rm -rf /tmp/gcc-$GCC_TAG
