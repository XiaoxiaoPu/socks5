/*
 * main.c
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

#include <libmill.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include "proxy.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif


static void help(void);
static void signal_cb(int signo);


int main(int argc, char **argv)
{
	const char *host = "0.0.0.0";
	int port = 1080;
	int nproc = 1;
	for (int i = 1; i < argc; i++)
	{
		if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0))
		{
			help();
			return EXIT_FAILURE;
		}
		else if (strcmp(argv[i], "-w") == 0)
		{
            if (i + 2 > argc)
			{
				fprintf(stderr, "missing value after '%s'\n", argv[i]);
				return EXIT_FAILURE;
			}
			nproc = atoi(argv[i + 1]);
			i++;
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
			port = atoi(argv[i + 1]);
			i++;
		}
		else
		{
			fprintf(stderr, "invalid option: %s\n", argv[i]);
			return EXIT_FAILURE;
		}
	}

	// register signal handler
#ifdef HAVE_SIGACTION
    struct sigaction sa;
    sa.sa_handler = signal_cb;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);
#else
    signal(SIGINT, signal_cb);
    signal(SIGTERM, signal_cb);
    signal(SIGHUP, signal_cb);
#endif

	// listen
	ipaddr addr = iplocal(host, port, 0);
	tcpsock ls = tcplisten(addr, 32);
	if (ls == NULL)
	{
		ERROR("listen");
		return EXIT_FAILURE;
	}

    LOG("listen on %s:%d", host, port);

	// fork worker
	for (int i = 0; i < nproc - 1; i++)
	{
		pid_t pid = mfork();
        if (pid < 0)
		{
           	ERROR("Can't create new process");
           	return 1;
        }
        if (pid == 0)
		{
          	break;
        }
	}

	while (1)
	{
        tcpsock sock = tcpaccept(ls, -1);
        if (sock == NULL)
		{
            continue;
		}
        go(worker(sock));
    }
}


static void help(void)
{
	puts("usage: socks5 -a host -p port\n"
	       "  -h    show this help\n"
	       "  -w    number of workers\n"
	       "  -a    bind address\n"
	       "  -p    listen port");
}

static void signal_cb(int signo)
{
    (void)signo;
    exit(0);
}
