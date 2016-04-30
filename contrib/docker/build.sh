#!/usr/bin/env bash
#
# Build socks5 with musl(https://www.musl-libc.org/) and static linking.
#

set -e


function build_libmill() {
    curl -s -L https://github.com/sustrik/libmill/archive/master.tar.gz | tar -zxf -
    mv libmill-master libmill
    cd libmill
    ./autogen.sh
    export CC=musl-gcc
    ./configure --enable-shared=false
    make libmill.la
    cd ..
}

function build_socks5() {
    curl -s -L https://github.com/XiaoxiaoPu/socks5/archive/master.tar.gz | tar -zxf -
    mv socks5-master socks5
    cd socks5
    autoreconf -if
    export CPPFLAGS=-I$(pwd)/../libmill
    export LDFLAGS=-L$(pwd)/../libmill/.libs
    export CC=musl-gcc
    ./configure --prefix=/usr --sysconfdir=/etc --enable-static
    make
    strip src/socks5
    cd ..
}


build_libmill
build_sniproxy
cp socks5/src/socks5 socks5-bin
rm -rf libmill socks5
mv socks5-bin socks5
