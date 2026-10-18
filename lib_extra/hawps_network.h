/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _HAWPS_NETWORK_H
#define _HAWPS_NETWORK_H

#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

/* Macros
 */

#define MAX_RETRIES 4

#define MSG_SIZE    16
#define MSG_EST1    "I love you"
#define MSG_EST2    "It's mutual"

/* Constant defines
 */

/* Types
 */

struct Connection {
	struct addrinfo *ai;
	struct addrinfo *aip;
	char            *port;
	int              socket;
	char            *target;
	bool             target_serves;
};

/* Function declarations
 */

struct Connection
Connection_new();

int
Connection_bind(struct Connection          *c,
                char              *restrict target,
                char              *restrict port,
                const bool                  target_serves);

int
Connection_establish(struct Connection *c);

void
Connection_free(struct Connection *c);

/* Function definitions
 */

#ifdef HAWPS_IMPL

struct Connection
Connection_new() {
	struct Connection ret;

	memset(&ret, 0, sizeof(struct Connection));
	ret.socket = -1;

	return ret;
}

int
Connection_bind(struct Connection          *c,
                char              *restrict target,
                char              *restrict port,
                const bool                  target_serves) {
	int              e;
	struct addrinfo  hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = (target_serves ? 0 : AI_PASSIVE);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = 0;
	hints.ai_addrlen = 0;
	hints.ai_addr = NULL;
	hints.ai_canonname = NULL;
	hints.ai_next = NULL;

	e = getaddrinfo(target, port, &hints, &c->ai);
	if (e) {
		fprintf(stderr, "%s\n", gai_strerror(e));
		return 1;
	}

	for (c->aip = c->ai; c->aip != NULL; c->aip = c->aip->ai_next) {
		c->socket = socket(c->aip->ai_family,
		                   c->aip->ai_socktype,
		                   c->aip->ai_protocol);
		if (c->socket == -1) {
			continue;
		}

		if (target_serves) {
			if (connect(c->socket,
			            c->aip->ai_addr,
			            c->aip->ai_addrlen) == 0) {
				break;
			}
		} else {
			if (bind(c->socket,
			         c->aip->ai_addr,
			         c->aip->ai_addrlen) == 0) {
				break;
			}
		}

		close(c->socket);
	}

	if (c->aip == NULL) {
		fprintf(stderr, "Could not bind\n");
		return 2;
	}

	c->port = port;
	c->target = target;
	c->target_serves = target_serves;

	return 0;
}

int
Connection_establish(struct Connection *c) {
	struct sockaddr client_addr;
	socklen_t       client_addr_len = sizeof(client_addr);
	size_t          i;
	char            recvbuf[MSG_SIZE] = "\0";
	char            sendbuf[MSG_SIZE] = "\0";

	if (c->target_serves) {
		for (i = 0; i < MAX_RETRIES; i++) {
			strncpy(sendbuf, MSG_EST1, MSG_SIZE);
			send(c->socket, sendbuf, MSG_SIZE, 0);
			recv(c->socket, recvbuf, MSG_SIZE, 0);
		
			if (strcmp(recvbuf, MSG_EST2) == 0) {
				break;
			}
		}

		if (i >= MAX_RETRIES) {
			fprintf(stderr,
			        "Connection failed after %i retries\n",
			        MAX_RETRIES);
			return 1;
		}
	} else {
		recvfrom(c->socket, recvbuf, MSG_SIZE, 0, &client_addr, &client_addr_len);
		if (strcmp(recvbuf, MSG_EST1) != 0) {
			return 1;
		}

		if (connect(c->socket,
		            &client_addr,
		            client_addr_len) != 0) {
			fprintf(stderr, "Could not connect\n");
			return 2;
		}

		strncpy(sendbuf, MSG_EST2, MSG_SIZE);
		send(c->socket, sendbuf, MSG_SIZE, 0);
	}

	return 0;
}

void
Connection_free(struct Connection *c) {
	freeaddrinfo(c->ai);
	close(c->socket);
}

#endif /* HAWPS_IMPL */

#endif /* _HAWPS_NETWORK_H */
