#
# Dockerfile for socks5(https://github.com/XiaoxiaoPu/socks5)
#

FROM alpine
MAINTAINER Xiaoxiao <i@pxx.io>

COPY socks5 /socks5

EXPOSE 1080/tcp

ENTRYPOINT ["/socks5", "-w", "2"]
