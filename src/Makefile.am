AM_CFLAGS = -O3 -pipe -W -Wall -fno-strict-aliasing

bin_PROGRAMS = socks5

socks5_SOURCES = \
    log.c proxy.c socks5.c tcprelay.c main.c \
    log.h proxy.h socks5.h tcprelay.h
socks5_LDADD = $(LIB_MILL)
