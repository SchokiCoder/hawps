// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "net.h"
#include "world.h"

#define MAX_CLIENTS      1
#define STD_TICKRATE     24.0
#define STD_WORLD_WIDTH  40
#define STD_WORLD_HEIGHT 40

const char *APP_HELP = "Usage: " APP_NAME " [OPTIONS]\n"
"\n"
"Silly program to simulate physics in *very* convincing ways.\n"
"It'll be great. Trust me.\n"
"\n"
"Options:\n"
"\n"
"    -a --about\n"
"        prints program name, version, license and repository information then exits\n"
"\n"
"    -h --help\n"
"        prints this message then exits\n"
"\n"
"    --port\n"
"        sets the TCP port number\n"
"        default: " DEF_TO_STRING(STD_IP_PORT) "\n"
"\n"
"    --tickrate\n"
"        sets the tickrate (ticks per second), which effects visible speed\n"
"        default: " DEF_TO_STRING(STD_TICKRATE) "\n"
"\n"
"    -v --version\n"
"        prints version information then exits\n"
"\n"
"    --world_width\n"
"        sets the width of the world\n"
"        default: " DEF_TO_STRING(STD_WORLD_WIDTH) "\n"
"\n"
"    --world_height\n"
"        sets the height of the world\n"
"        default: " DEF_TO_STRING(STD_WORLD_HEIGHT) "\n"
"\n";

/* Returns 0 on success
 */
int
handle_args(
	int    argc,
	char  *argv[],
	int   *ip_port,
	float *tickrate,
	int   *world_w,
	int   *world_h);

int
handle_args(
	int    argc,
	char  *argv[],
	int   *ip_port,
	float *tickrate,
	int   *world_w,
	int   *world_h)
{
	const char *ERR_ARG_CONV =
		"\"%s\" could not be converted to a %s\n";
	const char *ERR_NO_ARG_VALUE =
		"The argument \"%s\" needs a value from the next argument\n";
	int i;
	int vi;
	float vf;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-a") == 0 ||
		    strcmp(argv[i], "--about") == 0) {
			printf("The source code of \"%s\" aka %s %s is available, "
			       "licensed under the %s at:\n"
			       "%s\n\n"
			       "If you did not receive a copy of the license, "
			       "see below:\n"
			       "%s\n",
			       APP_NAME_FORMAL, APP_NAME, APP_VERSION,
			       APP_LICENSE,
			       APP_REPOSITORY,
			       APP_LICENSE_URL);
			return 1;
		} else if (strcmp(argv[i], "-h") == 0 ||
		           strcmp(argv[i], "--help") == 0) {
			printf("%s", APP_HELP);
			return 1;
		} else if (strcmp(argv[i], "--port") == 0) {
			if (argc <= i + 1) {
				fprintf(stderr, ERR_NO_ARG_VALUE, argv[i]);
				return 0;
			}
			i++;

			errno = 0;
			vi = strtol(argv[i], NULL, 10);
			if (errno != 0 || 0 == vi) {
				fprintf(stderr,
				        ERR_ARG_CONV,
				        argv[i - 1],
				        "int");
				return 0;
			}
			*ip_port = vi;
		} else if (strcmp(argv[i], "--tickrate") == 0) {
			if (argc <= i + 1) {
				fprintf(stderr, ERR_NO_ARG_VALUE, argv[i]);
				return 1;
			}
			i++;

			errno = 0;
			vf = strtof(argv[i], NULL);
			if (errno != 0) {
				fprintf(stderr,
				        ERR_ARG_CONV,
				        argv[i - 1],
				        "float");
				return 1;
			}
			*tickrate = vf;
		} else if (strcmp(argv[i], "-v") == 0 ||
		           strcmp(argv[i], "--version") == 0) {
			printf("%s: version %s\n", APP_NAME, APP_VERSION);
			return 1;
		} else if (strcmp(argv[i], "--world_width") == 0) {
			if (argc <= i + 1) {
				fprintf(stderr, ERR_NO_ARG_VALUE, argv[i]);
				return 1;
			}
			i++;

			errno = 0;
			vi = strtol(argv[i], NULL, 10);
			if (errno != 0 || 0 == vi) {
				fprintf(stderr,
				        ERR_ARG_CONV,
				        argv[i - 1],
				        "int");
				return 1;
			}
			*world_w = vi;
		} else if (strcmp(argv[i], "--world_height") == 0) {
			if (argc <= i + 1) {
				fprintf(stderr, ERR_NO_ARG_VALUE, argv[i]);
				return 1;
			}
			i++;

			errno = 0;
			vi = strtol(argv[i], NULL, 10);
			if (errno != 0 || 0 == vi) {
				fprintf(stderr,
				        ERR_ARG_CONV,
				        argv[i - 1],
				        "int");
				return 1;
			}
			*world_h = vi;
		} else {
			fprintf(stderr,
			        "Argument \"%s\" is not recognized.\n",
			        argv[i]);
			return 1;
		}
	}

	return 0;
}

int
main(
	int argc,
	char *argv[])
{
	int                active = 1;
	int                csocket;
	float              delta;
	int                ip_port = STD_IP_PORT;
	struct sockaddr_in sockaddr;
	int                socket = -1;
	clock_t            t1, t2;
	float              tickrate = STD_TICKRATE;
	float              pause_mod = 1.0;
	struct World       wld;
	int                x, y;

	wld.w = STD_WORLD_WIDTH;
	wld.h = STD_WORLD_HEIGHT;

	if (handle_args(argc,
	                argv,
	                &ip_port,
			&tickrate,
			&wld.w,
			&wld.h) != 0) {
		return 0;
	}

	if (World_new(&wld, wld.w, wld.h) != 0) {
		fprintf(stderr, "Couldn't initialize world\n");
		goto cleanup;
	}

	// TODO: remove manual tomfoolery
	const int spawn1X = wld.w / 3;
	const int spawn1Y = wld.h / 3 * 2;
	const int spawn1W = 10;
	const int spawn1H = 10;
	const int spawn2X = wld.w / 3;
	const int spawn2Y = 0;
	const int spawn2W = wld.w / 4;
	const int spawn2H = wld.h / 3;
	for (x = spawn2X; x < spawn2X + spawn2W; x++) {
		for (y = spawn2Y; y < spawn2Y + spawn2H; y++) {
			wld.dots[x][y] = M_water;
		}
	}
	for (x = spawn1X; x < spawn1X + spawn1W; x++) {
		for (y = spawn1Y; y < spawn1Y + spawn1H; y++) {
			wld.dots[x][y] = M_sand;
		}
	}

	t1 = 0.0;
	t2 = 0.0;

	socket = TCP_open_socket();
	if (-1 == socket) {
		fprintf(stderr, "Socket init failed\n");
		goto cleanup;
	}

	if (TCP_setup_sockaddr(&sockaddr, NULL, ip_port) == -1) {
		fprintf(stderr, "Invalid IP address\n");
		goto cleanup;
	}

	if (bind(socket, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) < 0) {
		fprintf(stderr, "Bind failed\n");
		goto cleanup;
	}

	if (listen(socket, MAX_CLIENTS) < 0) {
		fprintf(stderr, "Listen failed\n");
		goto cleanup;
	}

	csocket = accept(socket, NULL, NULL);
	if (0 > csocket) {
		fprintf(stderr, "Accept failed\n");
		goto cleanup;
	}

	write(csocket, &wld.w, sizeof(wld.w));
	write(csocket, &wld.h, sizeof(wld.h));

	while (active) {
		t1 = clock();
		delta = 1.0 * (t1 - t2) / CLOCKS_PER_SEC;
		if (delta * pause_mod >= (1.0 / tickrate)) {
			World_tick(&wld);

			t2 = t1;

			if (write(csocket,
				  wld._dots_data,
				  sizeof(enum Mat) * wld.w * wld.h)
			    == -1) {
				active = 0;
				break;
			}
		}
	}

cleanup:
	close(socket);
	World_free(&wld);
}
