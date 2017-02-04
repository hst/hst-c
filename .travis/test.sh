#!/bin/sh
set -ev

srcdir="$PWD"

if [ ${BUILD_TYPE} = out_of_source ]; then
    mkdir .build
    cd .build
fi

${srcdir}/autogen.sh
if [ ${BUILD_TYPE} = hella_optimized ]; then
    ${srcdir}/configure --enable-valgrind \
        CPPFLAGS="-DNDEBUG" \
        CFLAGS="-O3 -flto -march=native" \
        LDFLAGS="-flto -march=native"
else
    ${srcdir}/configure --enable-valgrind CFLAGS="-g -Os"
fi
make
make check
make check-valgrind

# distcheck doesn't pass on our optimization settings, so we don't need to run
# it during the hella-optimized build.
if [ ${BUILD_TYPE} != hella_optimized ]; then
    make distcheck
fi
