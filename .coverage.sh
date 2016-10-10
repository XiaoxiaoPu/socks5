#!/usr/bin/env bash

set -e


# build libmill
rm -rf libmill
curl -s -L https://github.com/sustrik/libmill/archive/master.tar.gz | tar -zxf -
mv libmill-master libmill
cd libmill
./autogen.sh
./configure --enable-shared=false
make libmill.la
cd ../

# build with coverage
if [ -f Makefile ]; then
    make distclean
fi
autoreconf -if
export CPPFLAGS
CPPFLAGS=-I$(pwd)/libmill
export LDFLAGS
LDFLAGS=-L$(pwd)/libmill/.libs
export CFLAGS="-fprofile-arcs -ftest-coverage"
./configure --enable-debug
make

# run tests
src/socks5 -h || true
src/socks5 -a || true
src/socks5 -p || true
src/socks5 -w || true
src/socks5 --nothisoption || true
src/socks5 -a 127.0.0.1 -p 10080 -w 4 &
curl -vv --socks5-hostname 127.0.0.1:10080 http://github.com/
curl -vv --socks5-hostname 127.0.0.1:10080 https://github.com/
pkill -INT socks5

# send coverage report to codecov.io
bash <(curl -s https://codecov.io/bash)

# cleanup
make distclean
