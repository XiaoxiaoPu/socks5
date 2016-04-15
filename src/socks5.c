/*
 * socks5.c - SOCKS5 worker
 *
 * Copyright (C) 2014 - 2016, Xiaoxiao <i@pxx.io>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <libmill.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include "socks5.h"
#include "tcprelay.h"


int socks5_accept(tcpsock sock, char *host, int *port)
{
    int64_t deadline = now() + 10000;
    uint8_t buf[10];

    {
        // SOCKS5 CLIENT HELLO
        // +-----+----------+----------+
        // | VER | NMETHODS | METHODS  |
        // +-----+----------+----------+
        // |  1  |    1     | 1 to 255 |
        // +-----+----------+----------+
        int error = 0;
        uint8_t ver;
        tcprecv(sock, &ver, 1, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
        // version must be 5
        if (ver != 5)
        {
            error = 1;
            goto server_hello;
        }

        uint8_t nmethods;
        uint8_t methods[255];
        tcprecv(sock, &nmethods, 1, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
        tcprecv(sock, methods, nmethods, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
        int i;
        for (i = 0; i < (int)nmethods; i++)
        {
            if (methods[i] == 0x00)
            {
                break;
            }
        }
        if (i >= (int)nmethods)
        {
            error = 2;
        }

        // SOCKS5 SERVER HELLO
        // +-----+--------+
        // | VER | METHOD |
        // +-----+--------+
        // |  1  |   1    |
        // +-----+--------+
      server_hello:
        buf[0] = 0x05;
        if (error == 0)
        {
            buf[1] = 0x00;
        }
        else
        {
            buf[1] = 0xff;
        }
        tcpsend(sock, buf, 2, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
        tcpflush(sock, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
        if (error != 0)
        {
            goto cleanup;
        }
    }

    {
        // SOCKS5 CLIENT REQUEST
        // +-----+-----+-------+------+----------+----------+
        // | VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
        // +-----+-----+-------+------+----------+----------+
        // |  1  |  1  | X'00' |  1   | Variable |    2     |
        // +-----+-----+-------+------+----------+----------+
        int error = 0;
        uint8_t ver;
        tcprecv(sock, &ver, 1, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
        // version must be 5
        if (ver != 5)
        {
            error = 1;
            goto server_reply;
        }

        uint8_t cmd;
        tcprecv(sock, &cmd, 1, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
        // only CONNECT supported
        if (cmd != 0x01)
        {
            error = 2;
            goto server_reply;
        }

        uint8_t rsv;
        tcprecv(sock, &rsv, 1, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }

        uint8_t atyp;
        tcprecv(sock, &atyp, 1, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
        if (atyp == 0x01)
        {
            // IPv4 address
            uint8_t addr[4];
            tcprecv(sock, addr, 4, deadline);
            if (errno != 0)
            {
                goto cleanup;
            }
            inet_ntop(AF_INET, addr, host, INET_ADDRSTRLEN);
        }
        else if (atyp == 0x03)
        {
            // Domain name
            uint8_t len;
            tcprecv(sock, &len, 1, deadline);
            if (errno != 0)
            {
                goto cleanup;
            }
            uint8_t addr[256];
            tcprecv(sock, addr, len, deadline);
            if (errno != 0)
            {
                goto cleanup;
            }
            memcpy(host, addr, len);
            host[len] = '\0';
        }
        else if (atyp == 0x04)
        {
            // IPv6 address
            uint8_t addr[16];
            tcprecv(sock, addr, 16, deadline);
            if (errno != 0)
            {
                goto cleanup;
            }
            inet_ntop(AF_INET6, addr, host, INET6_ADDRSTRLEN);
        }
        else
        {
            // unsupported address type
            error = 3;
            goto server_reply;
        }

        uint16_t _port;
        tcprecv(sock, &_port, 2, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
        *port = ntohs(_port);

        // SOCKS5 SERVER REPLY
        // +-----+-----+-------+------+----------+----------+
        // | VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
        // +-----+-----+-------+------+----------+----------+
        // |  1  |  1  | X'00' |  1   | Variable |    2     |
        // +-----+-----+-------+------+----------+----------+
      server_reply:
        bzero(buf, 10);
        buf[0] = 0x05;
        if (error == 0)
        {
            buf[1] = 0x00;
        }
        else if (error == 1)
        {
            buf[1] = 0x01;
        }
        else if (error == 2)
        {
            buf[1] = 0x07;
        }
        else
        {
            buf[1] = 0x08;
        }
        buf[2] = 0x00;
        buf[3] = 0x01;
        tcpsend(sock, buf, 10, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
        tcpflush(sock, deadline);
        if (errno != 0)
        {
            goto cleanup;
        }
        if (error != 0)
        {
            goto cleanup;
        }
        return 0;
    }

  cleanup:
    tcpclose(sock);
    return -1;
}
