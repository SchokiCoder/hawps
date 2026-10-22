/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#ifndef _HAWPS_NETWORK_H
#define _HAWPS_NETWORK_H

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

/* Macros
 */

#define NET_MAX_CONNECT_RETRIES 4
#define NET_MEDIUM_MSG_SIZE     64 /* enough for type, version info | world info */
#define NET_RETRY_SLEEP_LEN     5  /* second */
#define NET_TICKRATE            30.0
#define NET_WORLD_AE_BYTES      8

/* Constant defines
 */

/* Types
 */

struct Client {
	struct addrinfo *ai;
	struct addrinfo *aip;
	char            *port;
	int              sockfd;
	char            *target;
};

struct Server {
	size_t           clients;
	struct sockaddr *cl_addr;
	socklen_t       *cl_addr_len;
	bool            *cl_connected;
	int              sockfd;
};

enum Message {
	MSG_CONNECT,
	MSG_CONNECTED,
	MSG_INCOMPATIBLE,
	MSG_NOSLOTS,

	MSG_REQ_WORLDINFO,

	/* Tells which array is being sent/received.
	 * Those commented out, don't need to exist,
	 * as they can be calculated on client side. */
	MSG_WAT_DISSOL,
	MSG_WAT_DOT,
	MSG_WAT_OXID,
	/* MSG_WAT_STATE, */
	MSG_WAT_THERMO,
	/* MSG_WAT_WEIGHT, */
};

/* Function declarations
 */

struct Client
Client_new();

int
Client_connect(struct Client *c,
               char          *restrict target,
               char          *restrict port);

void
Client_mainloop(struct Client *c,
                const bool    *active,
                struct World  *w);

void
Client_recv_worldarr(struct Client *c,
                     char          *buf,
                     const size_t   buf_size,
                     struct World  *w);

void
Client_free(struct Client *c);

struct Server
Server_new(size_t         max_clients,
           const uint16_t port);

void
Server_mainloop(struct Server *s,
                const bool    *active,
                struct World  *w);

void
Server_send_worldarr(struct Server      *s,
                     char               *buf,
                     const size_t        buf_size,
                     size_t              client_id,
                     void               *world_arr,
                     const size_t        world_arr_size,
                     const enum Message  msg_wat);

void
Server_free(struct Server *s);

/* Function definitions
 */

#ifdef HAWPS_IMPL

struct Client
Client_new() {
	struct Client ret;

	memset(&ret, 0, sizeof(ret));
	ret.sockfd = -1;

	return ret;
}

int
Client_connect(struct Client *c,
               char          *restrict target,
               char          *restrict port)
{
	int              e;
	struct addrinfo  hints;
	size_t           i;
	size_t           pos;
	char             recv_buf[NET_MEDIUM_MSG_SIZE] = "\0";
	enum Message     recv_msg;
	char             send_buf[NET_MEDIUM_MSG_SIZE] = "\0";
	enum Message     send_msg;

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = 0;
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
		c->sockfd = socket(c->aip->ai_family,
		                   c->aip->ai_socktype,
		                   c->aip->ai_protocol);
		if (c->sockfd == -1) {
			continue;
		}

		if (connect(c->sockfd,
		            c->aip->ai_addr,
		            c->aip->ai_addrlen) != 0) {
			close(c->sockfd);
			fprintf(stderr, "socket connect: %i\n", errno);
		} else {
			break;
		}
	}

	if (c->aip == NULL) {
		fprintf(stderr, "Could not connect\n");
		return 2;
	}

	c->port = port;
	c->target = target;

	for (i = 0; i < NET_MAX_CONNECT_RETRIES; i++) {
		send_msg = MSG_CONNECT;
		pos = 0;
		memcpy(&send_buf[pos], &send_msg, sizeof(send_msg));
		pos += sizeof(send_msg);
		strncpy(&send_buf[pos], APP_VERSION, NET_MEDIUM_MSG_SIZE - pos);
		send(c->sockfd, send_buf, NET_MEDIUM_MSG_SIZE, 0);

		if (recv(c->sockfd, &recv_msg, sizeof(recv_msg), MSG_DONTWAIT) > 0) {
			break;
		}
		sleep(NET_RETRY_SLEEP_LEN);
	}

	if (i >= NET_MAX_CONNECT_RETRIES) {
		fprintf(stderr,
		        "Connection failed after %i retries\n",
		        NET_MAX_CONNECT_RETRIES);
		return 3;
	}

	switch (recv_msg) {
	case MSG_CONNECTED:
		printf("Connected\n");
		return 0;
		break;

	case MSG_INCOMPATIBLE:
		fprintf(stderr, "Server version doesn't match\n");
		return 4;
		break;

	case MSG_NOSLOTS:
		fprintf(stderr, "Server has no slots available\n");
		return 5;
		break;

	default:
		fprintf(stderr, "Server did not respond properly\n");
		return 6;
		break;
	}

	return 0;
}

void
Client_mainloop(struct Client *c,
                const bool    *active,
                struct World  *w)
{
	size_t        buf_size = NET_MEDIUM_MSG_SIZE;
	char         *buf = malloc(buf_size);
	clock_t       last_tick = 0;
	clock_t       now = 0;
	size_t        pos;
	enum Message  send_msg;

	if (w->w == 0 || w->h == 0) {
		send_msg = MSG_REQ_WORLDINFO;
		send(c->sockfd, &send_msg, sizeof(send_msg), 0);

		recv(c->sockfd, buf, buf_size, 0);
		w->w = buf[0];
		w->h = buf[sizeof(w->w)];
		*w = World_new(w->w, w->h);
	}

	buf_size = sizeof(enum Message) +
	           (NET_WORLD_AE_BYTES * w->w * w->h) +
	           1;
	if (buf_size < NET_MEDIUM_MSG_SIZE) {
		buf_size = NET_MEDIUM_MSG_SIZE;
	}
	buf = realloc(buf, buf_size);
	while (*active) {
		now = clock();
		if (now - last_tick < (long) (CLOCKS_PER_SEC / NET_TICKRATE)) {
			continue;
		}
		last_tick = now;

		Client_recv_worldarr(c, buf, buf_size, w);
	}
}

void
Client_recv_worldarr(struct Client *c,
                     char          *buf,
                     const size_t   buf_size,
                     struct World  *w)
{
	enum Message msg_wat;
	size_t       pos;

	recv(c->sockfd, buf, buf_size, 0);
	msg_wat = buf[0];
	pos = sizeof(msg_wat);

	switch (msg_wat) {
	case MSG_WAT_DISSOL:
		memcpy(w->_dissol, &buf[pos], w->w * w->h * sizeof(*w->_dissol));
		break;

	case MSG_WAT_DOT:
		memcpy(w->_dot, &buf[pos], w->w * w->h * sizeof(*w->_dot));
		break;

	case MSG_WAT_OXID:
		memcpy(w->_oxid, &buf[pos], w->w * w->h * sizeof(*w->_oxid));
		break;

	case MSG_WAT_THERMO:
		memcpy(w->_thermo, &buf[pos], w->w * w->h * sizeof(*w->_thermo));
		break;

	default:
		break;
	}
}

void
Client_free(struct Client *c)
{
	freeaddrinfo(c->ai);
	close(c->sockfd);
}

struct Server
Server_new(size_t         max_clients,
           const uint16_t port) {
	struct sockaddr_in6 addr = {
		.sin6_family = AF_INET6,
		.sin6_port = htons(port),
		.sin6_addr = in6addr_any,
	};
	struct Server ret;
	int val = 0;

	memset(&ret, 0, sizeof(ret));

	ret.sockfd = socket(AF_INET6, SOCK_DGRAM, 0);
	if (ret.sockfd == -1) {
		fprintf(stderr, "Couldn't get socket: %i\n", errno);
		return ret;
	}
	if (setsockopt(ret.sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof(val))) {
		fprintf(stderr, "Couldn't set IPv& only: %i\n", errno);
		return ret;
	}
	if (bind(ret.sockfd, (struct sockaddr*) &addr, sizeof(addr))) {
		fprintf(stderr, "Couldn't bind: %i\n", errno);
		return ret;
	}

	ret.clients = max_clients;
	ret.cl_addr = calloc(ret.clients, sizeof(struct sockaddr));
	ret.cl_addr_len = calloc(ret.clients, sizeof(socklen_t));
	ret.cl_connected = calloc(ret.clients, sizeof(bool));

	return ret;
}

void
Server_handle_message(struct Server         *s,
                      const struct sockaddr  recv_addr,
                      const socklen_t        recv_addr_len,
                      const char            *recv_buf,
                      const struct World     w)
{
	size_t           i;
	char             ip[INET_ADDRSTRLEN];
	size_t           pos;
	enum Message     recv_msg;
	char             send_buf[NET_MEDIUM_MSG_SIZE];
	enum Message     send_msg;

	recv_msg = recv_buf[0];
	switch (recv_msg) {
	case MSG_CONNECT:
		if (strcmp(&recv_buf[sizeof(recv_msg)], APP_VERSION) != 0) {
			send_msg = MSG_INCOMPATIBLE;
			sendto(s->sockfd, &send_msg, sizeof(send_msg), 0,
			       &recv_addr, recv_addr_len);
			fprintf(stderr, "Client version doesn't match\n");
			break;
		}

		for (i = 0; i < s->clients; i++) {
			if (s->cl_connected[i] == true) {
				continue;
			}

			inet_ntop(AF_INET, &recv_addr, ip, INET_ADDRSTRLEN);
			printf("Connection established with %s\n", ip);
			s->cl_addr[i] = recv_addr;
			s->cl_addr_len[i] = recv_addr_len;
			s->cl_connected[i] = true;
			send_msg = MSG_CONNECTED;
			sendto(s->sockfd, &send_msg, sizeof(send_msg), 0,
			       &recv_addr, recv_addr_len);
			break;
		}

		if (i >= s->clients) {
			send_msg = MSG_NOSLOTS;
			sendto(s->sockfd, &send_msg, sizeof(send_msg), 0,
			       &recv_addr, recv_addr_len);
		}
		break;

	case MSG_REQ_WORLDINFO:
		pos = 0;
		memcpy(&send_buf[pos], &w.w, sizeof(w.w));
		pos += sizeof(w.w);
		memcpy(&send_buf[pos], &w.h, sizeof(w.h));
		pos += sizeof(w.h);
		send_buf[pos] = '\0';
		sendto(s->sockfd, send_buf, NET_MEDIUM_MSG_SIZE, 0,
		       &recv_addr, recv_addr_len);
		break;

	default:
		// TODO add counter for invalid messages,
		// and disconnect clients that keep sending them
		break;
	}
}

void
Server_mainloop(struct Server *s,
                const bool    *active,
                struct World  *w)
{
	size_t           i;
	clock_t          last_tick = 0;
	clock_t          now = 0;
	struct sockaddr  recv_addr;
	socklen_t        recv_addr_len;
	char             recv_buf[NET_MEDIUM_MSG_SIZE];
	size_t           send_buf_size = 0;
	char            *send_buf = NULL;

	send_buf_size = sizeof(enum Message) +
	                (NET_WORLD_AE_BYTES * w->w * w->h) +
	                1;
	if (send_buf_size < sizeof(enum Message)) {
		send_buf_size = sizeof(enum Message);
	}
	send_buf = malloc(send_buf_size);

	while (*active) {
		now = clock();
		if (now - last_tick < (long) (CLOCKS_PER_SEC / NET_TICKRATE)) {
			continue;
		}
		last_tick = now;

		if (recvfrom(s->sockfd,
		             &recv_buf,
		             NET_MEDIUM_MSG_SIZE,
		             MSG_DONTWAIT,
		             &recv_addr,
		             &recv_addr_len) > 0) {
			Server_handle_message(s,
			                      recv_addr,
			                      recv_addr_len,
			                      recv_buf,
			                      *w);
		}

		for (i = 0; i < s->clients; i++) {
			if (!s->cl_connected[i]) {
				continue;
			}

			Server_send_worldarr(s,
			                     send_buf,
			                     send_buf_size,
			                     i,
			                     w->_dissol,
			                     w->w * w->h * sizeof(*w->_dissol),
			                     MSG_WAT_DISSOL);
			Server_send_worldarr(s,
			                     send_buf,
			                     send_buf_size,
			                     i,
			                     w->_dot,
			                     w->w * w->h * sizeof(*w->_dot),
			                     MSG_WAT_DOT);
			Server_send_worldarr(s,
			                     send_buf,
			                     send_buf_size,
			                     i,
			                     w->_oxid,
			                     w->w * w->h * sizeof(*w->_oxid),
			                     MSG_WAT_OXID);
			Server_send_worldarr(s,
			                     send_buf,
			                     send_buf_size,
			                     i,
			                     w->_thermo,
			                     w->w * w->h * sizeof(*w->_thermo),
			                     MSG_WAT_THERMO);
		}
	}
}

void
Server_send_worldarr(struct Server      *s,
                     char               *buf,
                     const size_t        buf_size,
                     size_t              client_id,
                     void               *world_arr,
                     const size_t        world_arr_size,
                     const enum Message  msg_wat)
{
	size_t len;
	size_t pos;

	pos = 0;
	len = sizeof(msg_wat);
	memset(buf, msg_wat, len);
	pos += len;
	len = world_arr_size;
	memcpy(&buf[pos], world_arr, len);
	pos += len;
	buf[pos] = '\0';
	sendto(s->sockfd, buf, buf_size, 0,
	       &s->cl_addr[client_id], s->cl_addr_len[client_id]);
}

void
Server_free(struct Server *s)
{
	if (s->cl_addr != NULL) {
		free(s->cl_addr);
	}
	if (s->cl_addr_len != NULL) {
		free(s->cl_addr_len);
	}
	if (s->cl_connected != NULL) {
		free(s->cl_connected);
	}
	s->clients = 0;
	close(s->sockfd);
}

#endif /* HAWPS_IMPL */

#endif /* _HAWPS_NETWORK_H */
