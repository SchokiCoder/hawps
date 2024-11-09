/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (C) 2024  Andy Frank Schoknecht
 */

#include "net.h"

#include <string.h>

int
TCP_open_socket(
	void)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > sock) {
		return -1;
	}

	return sock;
}

int
TCP_setup_sockaddr(
	struct sockaddr_in *sockaddr,
	const char         *ip_address)
{
	memset(sockaddr, 0, sizeof(*sockaddr));
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_port = htons(IP_PORT);

	if (NULL == ip_address) {
		sockaddr->sin_addr.s_addr = htonl(INADDR_ANY);
	} else {
		if (inet_aton(ip_address, &sockaddr->sin_addr) <= 0) {
			return 1;
		}
	}

	return 0;
}
