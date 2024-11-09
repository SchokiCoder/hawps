/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (C) 2024  Andy Frank Schoknecht
 */

#ifndef _NET_H
#define _NET_H

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

int
TCP_open_socket(
	void);

int
TCP_setup_sockaddr(
	struct sockaddr_in *sockaddr,
	const char         *ip_address);

#endif /* _NET_H */
