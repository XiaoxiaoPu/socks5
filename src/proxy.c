/*
 * proxy.c - proxy worker
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

#include <errno.h>
#include <libmill.h>
#include <stdint.h>
#include <string.h>
#include "log.h"
#include "socks5.h"
#include "tcprelay.h"


coroutine void worker(tcpsock sock)
{
    int64_t deadline = now() + 10000;

    // get socks5 request
    char host[257];
    int port;
    if (socks5_accept(sock, host, &port) != 0)
    {
        return;
    }

    LOG("connect %s:%d", host, port);

    ipaddr addr = ipremote(host, port, IPADDR_PREF_IPV4, deadline);
    if (errno != 0)
    {
        tcpclose(sock);
        return;
    }

    tcpsock conn = tcpconnect(addr, deadline);
    if (errno != 0)
    {
        tcpclose(sock);
        return;
    }

    tcprelay(sock, conn);
    LOG("connection closed");
}
