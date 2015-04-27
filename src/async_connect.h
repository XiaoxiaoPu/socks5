/*
 * async_connect.h - async connect
 *
 * Copyright (C) 2014 - 2015, Xiaoxiao <i@xiaoxiao.im>
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

#ifndef ASYNC_CONNECT_H
#define ASYNC_CONNECT_H

#include <sys/socket.h>

extern void async_connect(const struct sockaddr *addr, socklen_t addrlen,
                          void (*cb)(int, void *), void *data);

#endif // ASYNC_CONNECT_H
