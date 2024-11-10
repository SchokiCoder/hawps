// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2024  Andy Frank Schoknecht

#include <errno.h>
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "net.h"
#include "world.h"

#define STD_IP_ADDRESS   "127.0.0.1"
#define STD_WORLD_SCALE  10

const char *APP_HELP =  "Usage: " APP_NAME " [OPTIONS]\n"
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
"    --ipv4\n"
"        sets the target IPv4 address in the format xxx.xxx.xxx.xxx\n"
"\n"
"    --port\n"
"        sets the TCP port number\n"
"        default: " DEF_TO_STRING(STD_IP_PORT) "\n"
"\n"
"    -v --version\n"
"        prints version information then exits\n"
"\n"
"    --world_scale\n"
"        sets the graphical scale of the physical world\n"
"        default: " DEF_TO_STRING(STD_WORLD_SCALE) "\n"
"\n"
"Default keybinds:\n"
"\n"
"    ESC\n"
"        quit the program\n"
"\n"
"    Space\n"
"        pause world\n"
"\n";

int
draw_world(
	const struct World  wld,
	SDL_Surface        *frame,
	SDL_Window         *win);

int
handle_args(
	int    argc,
	char  *argv[],
	char **ip_address,
	int   *ip_port,
	int   *wld_scale);

void
handle_events(
	int *active);

void
handle_key(
	SDL_Keycode key,
	int *active,
	float *pause_mod);

int
draw_world(
	const struct World  wld,
	SDL_Surface        *frame,
	SDL_Window         *win)
{
	Uint32 pixel;
	SDL_Rect rect;
	SDL_Surface *tmp;
	int x, y;

	for (x = 0; x < wld.w; x++) {
		for (y = 0; y < wld.h; y++) {
			rect.x = x;
			rect.y = y;
			rect.w = 1;
			rect.h = 1;

			pixel = SDL_MapRGB(frame->format,
			                   MAT_R[wld.dots[x][y]],
			                   MAT_G[wld.dots[x][y]],
			                   MAT_B[wld.dots[x][y]]);
			SDL_FillRect(frame, &rect, pixel);
		}
	}

	tmp = SDL_GetWindowSurface(win);
	if (NULL == tmp) {
		fprintf(stderr, "Window has no surface :O\n");
		return 1;
	}

	SDL_BlitScaled(frame, &frame->clip_rect, tmp, &tmp->clip_rect);
	SDL_UpdateWindowSurface(win);

	return 0;
}

int
handle_args(
	int    argc,
	char  *argv[],
	char **ip_address,
	int   *ip_port,
	int   *wld_scale)
{
	const char *ERR_ARG_CONV =
		"\"%s\" could not be converted to a %s\n";
	const char *ERR_NO_ARG_VALUE =
		"The argument \"%s\" needs a value from the next argument\n";
	int i;
	int vi;

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
			return 0;
		} else if (strcmp(argv[i], "-h") == 0 ||
		           strcmp(argv[i], "--help") == 0) {
			printf("%s", APP_HELP);
			return 0;
		} else if (strcmp(argv[i], "--ipv4") == 0) {
			if (argc <= i + 1) {
				fprintf(stderr, ERR_NO_ARG_VALUE, argv[i]);
				return 0;
			}
			i++;

			*ip_address = argv[i];
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
		} else if (strcmp(argv[i], "-v") == 0 ||
		           strcmp(argv[i], "--version") == 0) {
			printf("%s: version %s\n", APP_NAME, APP_VERSION);
			return 0;
		} else if (strcmp(argv[i], "--world_scale") == 0) {
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
			*wld_scale = vi;
		} else {
			fprintf(stderr,
			        "Argument \"%s\" is not recognized.\n",
			        argv[i]);
			return 0;
		}
	}

	return 1;
}

void
handle_events(
	int *active)
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYUP:
			//write(socket, CE_request_pause, sizeof(CE_request_pause));
			break;

		case SDL_QUIT:
			*active = 0;
			break;

		default:
			break;
		}
	}
}

void
handle_key(
	SDL_Keycode key,
	int *active,
	float *pause_mod)
{
	switch (key) {
	case SDLK_ESCAPE:
		*active = 0;
		break;

	case SDLK_SPACE:
		if (*pause_mod != 0.0) {
			*pause_mod = 0.0;
		} else {
			*pause_mod = 1.0;
		}
		break;

	default:
		break;
	}
}

int
main(
	int argc,
	char *argv[])
{
	int                 active = 1;
	SDL_Surface        *frame = NULL;
	char               *ip_address = STD_IP_ADDRESS;
	int                ip_port = STD_IP_PORT;
	struct sockaddr_in  sockaddr;
	int                 socket = -1;
	SDL_Window         *win = NULL;
	int                 wld_scale = STD_WORLD_SCALE;
	struct World        wld;

	if (handle_args(argc,
	                argv,
	                &ip_address,
	                &ip_port,
	                &wld_scale)
	    == 0) {
		return 0;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "SDL Init failed\n");
		return 0;
	}

	socket = TCP_open_socket();
	if (-1 == socket) {
		fprintf(stderr, "Socket init failed\n");
		goto cleanup;
	}

	if (TCP_setup_sockaddr(&sockaddr, ip_address, ip_port) == -1) {
		fprintf(stderr, "Invalid IP address\n");
		goto cleanup;
	}

	if (connect(socket, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) < 0) {
		fprintf(stderr, "Connect failed\n");
		goto cleanup;
	}

	read(socket, &wld.w, sizeof(wld.w));
	read(socket, &wld.h, sizeof(wld.h));

	if (World_new(&wld, wld.w, wld.h) != 0) {
		fprintf(stderr, "Couldn't initialize world\n");
		goto cleanup;
	}

	win = SDL_CreateWindow(APP_NAME_FORMAL,
	                       SDL_WINDOWPOS_UNDEFINED,
	                       SDL_WINDOWPOS_UNDEFINED,
	                       wld.w * wld_scale,
	                       wld.h * wld_scale,
	                       SDL_WINDOW_SHOWN);
	if (NULL == win) {
		fprintf(stderr, "Couldn't open window\n");
		goto cleanup;
	}

	frame = SDL_CreateRGBSurface(0, wld.w, wld.h, 32,
	                             0, 0, 0, 0);
	if (NULL == frame) {
		fprintf(stderr, "Couldn't create surface\n");
		goto cleanup;
	}

	while (active) {
		handle_events(&active);

		if (read(socket,
		         wld._dots_data,
		         sizeof(enum Mat) * wld.w * wld.h)
		    == -1) {
			active = 0;
			break;
		}

		if (draw_world(wld, frame, win) != 0) {
			active = 0;
			break;
		}
	}

cleanup:
	SDL_FreeSurface(frame);
	SDL_DestroyWindow(win);
	World_free(&wld);
	close(socket);
	SDL_Quit();
}
