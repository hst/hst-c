#!/bin/sh
set -ev

srcdir="$PWD"

if [ ${BUILD_TYPE} = out_of_source ]; then
    mkdir .build
    cd .build
fi

${srcdir}/autogen.sh
${srcdir}/configure --enable-valgrind CFLAGS="-g -O3 -flto" LDFLAGS="-flto"
make
make check
make check-valgrind
make distcheck
