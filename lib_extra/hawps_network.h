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

#define MSG_SIZE          16              // <- visual max len
#define MSG_EST1          "I love you"
#define MSG_EST2          "It's mutual"
#define MSG_COMPATIBLE    "We're a match"
#define MSG_INCOMPATIBLE  "We don't fit"
#define MSG_WORLDINFO_PLS "Gime winfo pls"

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

/* Tells which array is being sent/received.
 * Those commented out, don't need to exist,
 * as they can be calculated on client side.
 */
enum MsgWorldArrtype {
	MSGWAT_DISSOL,
	MSGWAT_DOT,
	MSGWAT_OXID,
	/* MSGWAT_STATE, */
	MSGWAT_THERMO,
	/* MSGWAT_WEIGHT, */
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
Connection_recv_worldarr(struct Connection  *c,
                         char               *buf,
                         const size_t        buf_size,
                         struct World       *w);

void
Connection_run(struct Connection *c,
               const bool        *active, 
               struct World      *w);

void
Connection_send_worldarr(struct Connection         *c,
                         char                      *buf,
                         const size_t               buf_size,
                         const struct World         w,
                         char                      *world_arr,
                         const enum MsgWorldArrtype wat);

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

		strncpy(sendbuf, APP_VERSION, MSG_SIZE);
		send(c->socket, sendbuf, MSG_SIZE, 0);

		memset(recvbuf, 0, MSG_SIZE);
		recv(c->socket, recvbuf, MSG_SIZE, 0);
		if (strcmp(recvbuf, MSG_COMPATIBLE) != 0) {
			fprintf(stderr, "Server version doesn't match\n");
			return 2;
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

		recv(c->socket, recvbuf, MSG_SIZE, 0);
		if (strcmp(recvbuf, APP_VERSION) != 0) {
			strncpy(sendbuf, MSG_INCOMPATIBLE, MSG_SIZE);
			send(c->socket, sendbuf, MSG_SIZE, 0);
			fprintf(stderr, "Client version doesn't match\n");
			return 3;
		} else {
			strncpy(sendbuf, MSG_COMPATIBLE, MSG_SIZE);
			send(c->socket, sendbuf, MSG_SIZE, 0);
		}
	}

	return 0;
}

void
Connection_recv_worldarr(struct Connection  *c,
                         char               *buf,
                         const size_t        buf_size,
                         struct World       *w) {
	char                 temp;
	enum MsgWorldArrtype wat;
	size_t               wat_len = sizeof(wat);

	recv(c->socket, buf, buf_size, 0);
	temp = buf[wat_len];
	buf[wat_len] = '\0';
	wat = buf[0];
	buf[wat_len] = temp;

	switch (wat) {
	case MSGWAT_DISSOL:
		memcpy(&w->_dissol, &buf[wat_len], buf_size - wat_len);
		break;

	case MSGWAT_DOT:
		memcpy(&w->_dot, &buf[wat_len], buf_size - wat_len);
		break;

	case MSGWAT_OXID:
		memcpy(&w->_oxid, &buf[wat_len], buf_size - wat_len);
		break;

	case MSGWAT_THERMO:
		memcpy(&w->_thermo, &buf[wat_len], buf_size - wat_len);
		break;
	}
}

void
Connection_run(struct Connection *c,
               const bool        *active, 
               struct World      *w) {
	size_t  buf_size = 0;
	size_t  len;
	size_t  pos;
	char   *recvbuf = NULL;
	char   *sendbuf = NULL;

	buf_size = sizeof(enum MsgWorldArrtype) +
	           (sizeof(float) * w->w * w->h) +
	           1;
	if (buf_size < MSG_SIZE) {
		buf_size = MSG_SIZE;
	}

	recvbuf = malloc(buf_size);
	sendbuf = malloc(buf_size);

	while (*active) {
		if (c->target_serves) {
			if (w->w == 0 || w->h == 0) {
				strncpy(sendbuf, MSG_WORLDINFO_PLS, MSG_SIZE);
				send(c->socket, sendbuf, MSG_SIZE, 0);

				recv(c->socket, recvbuf, MSG_SIZE, 0);
				w->h = recvbuf[sizeof(w->w)];
				recvbuf[sizeof(w->w)] = '\0';
				w->h = recvbuf[0];
				*w = World_new(w->w, w->h);
			}

			Connection_recv_worldarr(c, recvbuf, buf_size, w);
		} else {
			if (recv(c->socket, recvbuf, MSG_SIZE, MSG_DONTWAIT) != 0 &&
			    strcmp(recvbuf, MSG_WORLDINFO_PLS) == 0) {
				pos = 0;
				memset(&sendbuf[pos], w->w, sizeof(w->w));
				pos += sizeof(w->w);
				memset(&sendbuf[pos], w->h, sizeof(w->h));
				pos += sizeof(w->h);
				sendbuf[pos] = '\0';
				send(c->socket, sendbuf, MSG_SIZE, 0);
			} else {
				Connection_send_worldarr(c,
				                         sendbuf,
				                         buf_size,
				                         *w,
				                         (char*) w->_dissol,
				                         MSGWAT_DISSOL);
				Connection_send_worldarr(c,
				                         sendbuf,
				                         buf_size,
				                         *w,
				                         (char*) w->_dot,
				                         MSGWAT_DOT);
				Connection_send_worldarr(c,
				                         sendbuf,
				                         buf_size,
				                         *w,
				                         (char*) w->_oxid,
				                         MSGWAT_OXID);
				Connection_send_worldarr(c,
				                         sendbuf,
				                         buf_size,
				                         *w,
				                         (char*) w->_thermo,
				                         MSGWAT_THERMO);
			}
		}
	}
}

void
Connection_send_worldarr(struct Connection         *c,
                         char                      *buf,
                         const size_t               buf_size,
                         const struct World         w,
                         char                      *world_arr,
                         const enum MsgWorldArrtype wat) {
	size_t len;
	size_t pos;

	len = sizeof(wat);
	memset(buf, wat, len);
	pos = len;
	len = w.w * w.h * sizeof(world_arr[0]);
	memcpy(&buf[pos], world_arr, len);
	pos = len;
	buf[pos] = '\0';
	send(c->socket, buf, buf_size, 0);
}

void
Connection_free(struct Connection *c) {
	freeaddrinfo(c->ai);
	close(c->socket);
}

#endif /* HAWPS_IMPL */

#endif /* _HAWPS_NETWORK_H */
