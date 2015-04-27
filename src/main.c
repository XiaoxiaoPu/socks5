/*
 * socks5.c - simple SOCKS5 server
 *
 * Copyright (C) 2014, Xiaoxiao <i@xiaoxiao.im>
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
#include <assert.h>
#include <ev.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "async_connect.h"
#include "async_resolv.h"
#include "log.h"
#include "relay.h"
#include "socks5.h"
#include "utils.h"

#define UNUSED(x) do {(void)(x);} while (0)

static void help(void);
static void signal_cb(EV_P_ ev_signal *w, int revents);
static void accept_cb(EV_P_ ev_io *w, int revents);
static void socks5_cb(int sock, char *host, char *port);
static void resolv_cb(struct addrinfo *res, void *data);
static void connect_cb(int sock, void *data);

// ev loop
struct ev_loop *loop;


int main(int argc, char **argv)
{
	char *host = NULL, *port = NULL;
	int daemon = 0;
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-h") == 0)
		{
			help();
			return EXIT_FAILURE;
		}
		else if (strcmp(argv[i], "-d") == 0)
		{
			daemon = 1;
		}
		else if (strcmp(argv[i], "-a") == 0)
		{
			if (i + 2 > argc)
			{
				fprintf(stderr, "missing value after '%s'\n", argv[i]);
				return EXIT_FAILURE;
			}
			host = argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-p") == 0)
		{
			if (i + 2 > argc)
			{
				fprintf(stderr, "missing value after '%s'\n", argv[i]);
				return EXIT_FAILURE;
			}
			port = argv[i + 1];
			i++;
		}
		else
		{
			fprintf(stderr, "invalid option: %s\n", argv[i]);
			return EXIT_FAILURE;
		}
	}
	if (host == NULL || port == NULL)
	{
		help();
		return EXIT_FAILURE;
	}

	// 初始化本地监听 socket
	struct addrinfo hints;
	struct addrinfo *res;
	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(host, port, &hints, &res) != 0)
	{
		LOG("failed to resolv %s:%s", host, port);
		return EXIT_FAILURE;
	}
	int sock = socket(res->ai_family, SOCK_STREAM, IPPROTO_TCP);
	if (sock < 0)
	{
		ERROR("socket");
		return EXIT_FAILURE;
	}
	setnonblock(sock);
	setreuseaddr(sock);
	if (bind(sock, (struct sockaddr *)res->ai_addr, res->ai_addrlen) != 0)
	{
		ERROR("bind");
		return EXIT_FAILURE;
	}
	freeaddrinfo(res);
	if (listen(sock, 1024) != 0)
	{
		ERROR("listen");
		return EXIT_FAILURE;
	}

	// 初始化 ev
	loop = EV_DEFAULT;
	ev_signal w_sigint;
	ev_signal w_sigterm;
	ev_signal_init(&w_sigint, signal_cb, SIGINT);
	ev_signal_init(&w_sigterm, signal_cb, SIGTERM);
	ev_signal_start(EV_A_ &w_sigint);
	ev_signal_start(EV_A_ &w_sigterm);
	ev_io w_listen;
	ev_io_init(&w_listen, accept_cb, sock, EV_READ);
	ev_io_start(EV_A_ &w_listen);

	// 初始化 async_resolv
	if (resolv_init() != 0)
	{
		return EXIT_FAILURE;
	}

	// 执行事件循环
	LOG("starting socks5 at %s:%s", host, port);
	ev_run(EV_A_ 0);

	// 退出
	close(sock);
	LOG("exit");

	return EXIT_SUCCESS;
}


static void help(void)
{
	puts("usage: socks5 -d host port\n"
	       "  -h    show this help\n"
	       "  -d    daemonize after startup\n"
	       "  -a    bind address\n"
	       "  -p    listen port");
}

static void signal_cb(EV_P_ ev_signal *w, int revents)
{
	UNUSED(revents);
	assert((w->signum == SIGINT) || (w->signum == SIGTERM));
	ev_break(EV_A_ EVBREAK_ALL);
}

static void accept_cb(EV_P_ ev_io *w, int revents)
{
	UNUSED(loop);
	UNUSED(revents);

	int sock = accept(w->fd, NULL, NULL);

	if (sock < 0)
	{
		ERROR("accept");
	}
	else
	{
		setnonblock(sock);
		setkeepalive(sock);
		socks5_accept(sock, socks5_cb);
	}
}

static void socks5_cb(int sock, char *host, char *port)
{
	LOG("connect %s:%s", host, port);
	async_resolv(host, port, resolv_cb, (void *)(uintptr_t)sock);
}

typedef struct
{
	int sock;
	struct addrinfo *_res;
	struct addrinfo *res;
} ctx_t;

static void resolv_cb(struct addrinfo *res, void *data)
{
	if (res != NULL)
	{
		// 域名解析成功，建立远程连接
		ctx_t *ctx = (ctx_t *)malloc(sizeof(ctx_t));
		if (ctx == NULL)
		{
			LOG("out of memory");
			close((int)(uintptr_t)data);
			return;
		}
		ctx->sock = (int)(uintptr_t)data;
		ctx->_res = res;
		ctx->res = res;
		async_connect(ctx->res->ai_addr, ctx->res->ai_addrlen, connect_cb, ctx);
	}
	else
	{
		// 域名解析失败
		close((int)(uintptr_t)data);
	}
}

static void connect_cb(int sock, void *data)
{
	ctx_t *ctx = (ctx_t *)(data);

	assert(ctx != NULL);

	if (sock > 0)
	{
		// 连接成功
		freeaddrinfo(ctx->_res);
		relay(ctx->sock, sock);
		free(ctx);
	}
	else
	{
		// 连接失败
		ctx->res = ctx->res->ai_next;
		if (ctx->res != NULL)
		{
			// 尝试连接下一个地址]
			async_connect(ctx->res->ai_addr, ctx->res->ai_addrlen, connect_cb, data);
		}
		else
		{
			// 所有地址均连接失败
			LOG("connect failed");
			close(ctx->sock);
			freeaddrinfo(ctx->_res);
			free(ctx);
		}
	}
}
