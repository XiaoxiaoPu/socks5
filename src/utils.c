/*
 * utils.c - Some util functions
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

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include "utils.h"

int setnonblock(int fd)
{
	int flags;
	flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		return -1;
	}
	if (-1 == fcntl(fd, F_SETFL, flags | O_NONBLOCK))
	{
		return -1;
	}
	return 0;
}

int setreuseaddr(int fd)
{
	int reuseaddr = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) != 0)
	{
		return -1;
	}
	return 0;
}

int setkeepalive(int fd)
{
	int keepalive = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int)) != 0)
	{
		return -1;
	}
	return 0;
}

int getsockerror(int fd)
{
	int error = 0;
	socklen_t len = sizeof(int);
	if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len) != 0)
	{
		return -1;
	}
	return error;
}

