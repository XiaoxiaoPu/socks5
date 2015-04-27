AM_CFLAGS=-pipe -W -Wall -fno-strict-aliasing

bin_PROGRAMS = socks5

socks5_SOURCES = \
    async_connect.c async_resolv.c log.c main.c relay.c socks5.c utils.c \
    async_connect.h async_resolv.h log.h relay.h socks5.h utils.h
socks5_LDADD = $(LIB_EV) $(LIB_ANL)
