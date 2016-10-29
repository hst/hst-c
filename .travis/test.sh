#!/bin/sh
set -ev
mkdir .build
cd .build
../autogen.sh
../configure --enable-valgrind CFLAGS="-g -O3 -flto" LDFLAGS="-flto"
make
make check
make check-valgrind
