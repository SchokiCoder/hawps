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

#define NET_CLIENT_LIFESIGNALRATE       0.33
#define NET_IPSTR_LEN                   INET6_ADDRSTRLEN
#define NET_MAX_CONNECT_RETRIES         4
#define NET_MEDIUM_MSG_SIZE             64   /* enough for type, version info | world info */
#define NET_RETRY_SLEEP_LEN             5    /* seconds */
#define NET_SERVER_HOUSEKEEPRATE        0.2
#define NET_SERVER_SLOT_DEACTIVATE_TIME 30
#define NET_SERVER_SLOT_DISCONNECT_TIME 20   /* seconds */
#define NET_SERVER_SLOTS_MULTIPLIER     2.0
#define NET_TICKRATE                    30.0
#define NET_UPDATERATE                  1.0
#define NET_WORLD_AE_BYTES              8

/* Constant defines
 */

#define NANOS_PER_SEC ((uint64_t) 1000000000)

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
	size_t           max_clients;
	size_t           cl_slots;
	bool            *cl_active;
	struct sockaddr *cl_addr;
	socklen_t       *cl_addr_len;
	bool            *cl_connected;
	char           **cl_ip;
	uint64_t        *cl_last_seen;
	uint64_t        *cl_last_update;
	int              sockfd;
};

enum Message {
	MSG_CONNECT,
	MSG_CONNECTED,
	MSG_INCOMPATIBLE,
	MSG_NOSLOTS,

	MSG_REQ_WORLDINFO,
	MSG_STILL_ALIVE,

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

/* @s: Server
 * @addr: addr from a recv call
 * @addr_len: addr length
 *
 * Returns client id or -1 upon failure.
 * Check with if (ret > Server.cl_slots).
 */
size_t
Server_get_clientid(struct Server         *s,
                    const struct sockaddr  addr,
                    const socklen_t        addr_len);

void
Server_handle_message(struct Server         *s,
                      const uint64_t         now,
                      const struct sockaddr  recv_addr,
                      const socklen_t        recv_addr_len,
                      const char            *recv_buf,
                      const struct World     w);

void
Server_housekeeping(struct Server  *s,
                    const uint64_t  now);

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
		send_msg = MSG_CONNECTED;
		send(c->sockfd, &send_msg, sizeof(send_msg), 0);
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
	size_t           buf_size = NET_MEDIUM_MSG_SIZE;
	char            *buf = malloc(buf_size);
	uint64_t         last_lifesignal = 0;
	uint64_t         last_tick = 0;
	uint64_t         now = 0;
	size_t           pos;
	enum Message     send_msg;
	struct timespec  ts;

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
		clock_gettime(CLOCK_MONOTONIC, &ts);
		now = ((uint64_t) ts.tv_sec * NANOS_PER_SEC) + (uint64_t) ts.tv_nsec;
		if (now - last_tick < (uint64_t) (NANOS_PER_SEC / NET_TICKRATE)) {
			continue;
		}
		last_tick = now;

		Client_recv_worldarr(c, buf, buf_size, w);

		if (now - last_lifesignal >=
		    (uint64_t) (NANOS_PER_SEC / NET_CLIENT_LIFESIGNALRATE)) {
			last_lifesignal = now;

			send_msg = MSG_STILL_ALIVE;
			send(c->sockfd, &send_msg, sizeof(send_msg), 0);
		}
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
	size_t i;
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

	ret.max_clients = max_clients;
	ret.cl_slots = max_clients * NET_SERVER_SLOTS_MULTIPLIER;
	ret.cl_active = calloc(ret.cl_slots, sizeof(bool));
	ret.cl_addr = calloc(ret.cl_slots, sizeof(struct sockaddr));
	ret.cl_addr_len = calloc(ret.cl_slots, sizeof(socklen_t));
	ret.cl_connected = calloc(ret.cl_slots, sizeof(bool));
	ret.cl_ip = calloc(ret.cl_slots, sizeof(char*));
	ret.cl_last_seen = calloc(ret.cl_slots, sizeof(uint64_t));
	ret.cl_last_update = calloc(ret.cl_slots, sizeof(uint64_t));

	for (i = 0; i < ret.cl_slots; i++) {
		ret.cl_ip[i] = calloc(NET_IPSTR_LEN, sizeof(char));
	}

	return ret;
}

size_t
Server_get_clientid(struct Server         *s,
                    const struct sockaddr  addr,
                    const socklen_t        addr_len) {
	size_t i;

	for (i = 0; i < s->cl_slots; i++) {
		if (addr_len == s->cl_addr_len[i] &&
		    addr.sa_family == s->cl_addr[i].sa_family &&
		    strcmp(addr.sa_data, s->cl_addr[i].sa_data) == 0) {
			return i;
		}
	}

	return -1;
}

void
Server_handle_message(struct Server         *s,
                      const uint64_t         now,
                      const struct sockaddr  recv_addr,
                      const socklen_t        recv_addr_len,
                      const char            *recv_buf,
                      const struct World     w)
{
	size_t           i;
	char             ip[NET_IPSTR_LEN];
	size_t           pos;
	enum Message     recv_msg;
	char             send_buf[NET_MEDIUM_MSG_SIZE];
	enum Message     send_msg;

	if (recv_addr.sa_family != AF_INET &&
	    recv_addr.sa_family != AF_INET6) {
		return;
	}

	recv_msg = recv_buf[0];
	switch (recv_msg) {
	case MSG_CONNECT:
		i = Server_get_clientid(s, recv_addr, recv_addr_len);
		if (i < s->cl_slots) {
			return;
		}

		if (strcmp(&recv_buf[sizeof(recv_msg)], APP_VERSION) != 0) {
			send_msg = MSG_INCOMPATIBLE;
			sendto(s->sockfd, &send_msg, sizeof(send_msg), 0,
			       &recv_addr, recv_addr_len);
			fprintf(stderr, "Client version doesn't match\n");
			break;
		}

		for (i = 0; i < s->cl_slots; i++) {
			if (s->cl_connected[i] == true) {
				continue;
			}

			inet_ntop(recv_addr.sa_family,
			          &recv_addr,
			          s->cl_ip[i],
			          NET_IPSTR_LEN);

			printf("Connection establishing with %s\n", s->cl_ip[i]);
			s->cl_addr[i] = recv_addr;
			s->cl_addr_len[i] = recv_addr_len;
			s->cl_connected[i] = true;
			send_msg = MSG_CONNECTED;
			sendto(s->sockfd, &send_msg, sizeof(send_msg), 0,
			       &recv_addr, recv_addr_len);
			break;
		}

		if (i >= s->cl_slots) {
			send_msg = MSG_NOSLOTS;
			sendto(s->sockfd, &send_msg, sizeof(send_msg), 0,
			       &recv_addr, recv_addr_len);
		}
		break;

	case MSG_CONNECTED:
		i = Server_get_clientid(s, recv_addr, recv_addr_len);
		if (i >= s->cl_slots) {
			inet_ntop(recv_addr.sa_family,
			          &recv_addr,
			          ip,
			          NET_IPSTR_LEN);
			printf("Invalid recipient %s\n", ip);
			return;
		}

		if (true == s->cl_active[i]) {
			return;
		}

		printf("Connection established with %s\n", s->cl_ip[i]);

		s->cl_active[i] = true;
		s->cl_last_seen[i] = now;
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

	case MSG_STILL_ALIVE:
		i = Server_get_clientid(s, recv_addr, recv_addr_len);
		if (i >= s->cl_slots) {
			inet_ntop(recv_addr.sa_family,
			          &recv_addr,
			          ip,
			          NET_IPSTR_LEN);
			printf("Invalid recipient %s\n", ip);
			return;
		}

		s->cl_last_seen[i] = now;
		break;

	default:
		// TODO add counter for invalid messages,
		// and disconnect clients that keep sending them
		break;
	}
}

void
Server_housekeeping(struct Server  *s,
                    const uint64_t  now)
{
	size_t i;

	for (i = 0; i < s->cl_slots; i++) {
		if (false == s->cl_connected[i]) {
			continue;
		}

		if (true == s->cl_active[i]) {
			if ((now - s->cl_last_seen[i]) / NANOS_PER_SEC >
			     NET_SERVER_SLOT_DEACTIVATE_TIME) {
				s->cl_active[i] = false;
				printf("No lifesignal from %s, disconnecting\n",
				       s->cl_ip[i]);
			}
		} else {
			if ((now - s->cl_last_seen[i]) / NANOS_PER_SEC >
			     NET_SERVER_SLOT_DISCONNECT_TIME) {
				memset(&s->cl_addr[i], 0, sizeof(s->cl_addr[i]));
				s->cl_active[i] = false;
				s->cl_addr_len[i] = 0;
				s->cl_connected[i] = false;
				s->cl_last_seen[i] = 0;
				s->cl_last_update[i] = 0;
			}
		}
	}
}

void
Server_mainloop(struct Server *s,
                const bool    *active,
                struct World  *w)
{
	size_t           i;
	uint64_t         last_housekeep = 0;
	uint64_t         last_tick = 0;
	uint64_t         now = 0;
	struct sockaddr  recv_addr;
	socklen_t        recv_addr_len = sizeof(recv_addr);
	char             recv_buf[NET_MEDIUM_MSG_SIZE];
	size_t           send_buf_size = 0;
	char            *send_buf = NULL;
	struct timespec  ts;

	send_buf_size = sizeof(enum Message) +
	                (NET_WORLD_AE_BYTES * w->w * w->h) +
	                1;
	if (send_buf_size < sizeof(enum Message)) {
		send_buf_size = sizeof(enum Message);
	}
	send_buf = malloc(send_buf_size);

	while (*active) {
		clock_gettime(CLOCK_MONOTONIC, &ts);
		now = ((uint64_t) ts.tv_sec * NANOS_PER_SEC) + (uint64_t) ts.tv_nsec;
		if (now - last_tick < (uint64_t) (NANOS_PER_SEC / NET_TICKRATE)) {
			continue;
		}
		last_tick = now;

		for (i = 0; i < s->cl_slots; i++) {
			if (!s->cl_connected[i] ||
			    (now - s->cl_last_update[i] <
			     (uint64_t) (NANOS_PER_SEC / NET_UPDATERATE))) {
				continue;
			}

			s->cl_last_update[i] = now;

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

		if (recvfrom(s->sockfd,
		             &recv_buf,
		             NET_MEDIUM_MSG_SIZE,
		             MSG_DONTWAIT,
		             &recv_addr,
		             &recv_addr_len) > 0) {
			Server_handle_message(s,
			                      now,
			                      recv_addr,
			                      recv_addr_len,
			                      recv_buf,
			                      *w);
		}

		if (now - last_housekeep <
		    (uint64_t) (NANOS_PER_SEC / NET_SERVER_HOUSEKEEPRATE)) {
			continue;
		}
		last_housekeep = now;
		Server_housekeeping(s, now);
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
	size_t i;

	if (s->cl_addr != NULL) {
		free(s->cl_addr);
	}
	if (s->cl_addr_len != NULL) {
		free(s->cl_addr_len);
	}
	if (s->cl_connected != NULL) {
		free(s->cl_connected);
	}
	if (s->cl_ip != NULL) {
		for (i = 0; i < s->cl_slots; i++) {
			free(s->cl_ip[i]);
		}
		free(s->cl_ip);
	}
	if (s->cl_last_update != NULL) {
		free(s->cl_last_update);
	}
	s->max_clients = 0;
	s->cl_slots = 0;
	close(s->sockfd);
}

#endif /* HAWPS_IMPL */

#endif /* _HAWPS_NETWORK_H */
