/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#define HAWPS_IMPL
#include <hawps_core.h>
#include <hawps_extra.h>
#include <pthread.h>

/* Macros
 */

/* Constant defines
 */

#define CMD_HELP       "help"
#define CMD_HELP_SHORT "h"
#define CMD_QUIT       "quit"
#define CMD_QUIT_SHORT "q"

#define FLAG_HELP       "-help"
#define FLAG_HELP_SHORT "-h"

#define SLOTS       16
#define TEMPERATURE (20.0 + CELSIUS_TO_KELVIN)
#define TICKRATE    24.0
#define PORT        25565
#define WORLD_W     24
#define WORLD_H     24

/* Types
 */

struct ThServerArgs {
	volatile bool *active;
	struct Server *sv;
	struct World  *w;
};

struct ThWorldArgs {
	volatile bool *active;
	struct World  *w;
};

/* Constants
 */

static const char APP_HELP[] = "Usage: " APP_NAME " [OPTIONS]\n"
"\n"
"Serious program to distribute *very* convincing physics worldwide.\n"
"Together we will cause a resonance cascade!\n"
"\n";

static const char APP_HELP_COMMANDS[] = "Commands:\n"
"\n"
"    " CMD_HELP_SHORT " " CMD_HELP "\n"
"        prints this text, duh\n"
"\n"
"    " CMD_QUIT_SHORT " " CMD_QUIT "\n"
"        gracefully shuts down the server\n"
"\n";

static const char APP_HELP_FLAGS[] = "Options:\n"
"\n"
"    " FLAG_HELP_SHORT " " FLAG_HELP "\n"
"        prints this message then exits\n"
"\n";

/* Function declarations
 */

/* Function definitions
 */

bool
handle_args(int    argc,
            char **argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], FLAG_HELP_SHORT) == 0 ||
		    strcmp(argv[i], FLAG_HELP) == 0) {
			printf(APP_HELP);
			printf(APP_HELP_FLAGS);
			printf(APP_HELP_COMMANDS);
			return false;
		}
	}

	return true;
}

void*
threadfn_server(void *arg)
{
	struct ThServerArgs *args = arg;
	uint64_t            last_tick = 0;
	uint64_t            now = 0;

	while (*args->active) {
		now = get_now();
		if (now - last_tick < (uint64_t) (NANOS_PER_SEC / TICKRATE)) {
			continue;
		}
		last_tick = now;

		Server_tick(args->sv, now, args->w);
	}

	return NULL;
}

void*
threadfn_world(void *arg)
{
	struct ThWorldArgs *args = arg;
	uint64_t            last_tick = 0;
	uint64_t            now = 0;

	while (*args->active) {
		now = get_now();
		if (now - last_tick < (uint64_t) (NANOS_PER_SEC / TICKRATE)) {
			continue;
		}
		last_tick = now;

		World_update(args->w, TEMPERATURE);
		World_sim(args->w);
	};

	return NULL;
}

int
main(int    argc,
     char **argv)
{
	volatile bool       active = true;
	size_t              i;
	char               *input = NULL;
	size_t              input_len = 64;
	pthread_t           th_sv = 0;
	struct ThServerArgs th_sva;
	pthread_t           th_w = 0;
	struct ThWorldArgs  th_wa;
	struct Server       sv;
	struct World        w;

	if (handle_args(argc, argv) == false) {
		return 0;
	}

	th_wa.active = &active;
	th_wa.w = &w;

	th_sva.active = &active;
	th_sva.sv = &sv;
	th_sva.w = &w;

	w = World_new(WORLD_W, WORLD_H);
	if (Server_new(&sv, SLOTS, PORT, WORLD_W, WORLD_H) != NE_OKAY) {
		fprintf(stderr, "Couldn't set up server\n");
		goto cleanup;
	}

	if (pthread_create(&th_w, NULL, threadfn_world, &th_wa) != 0 ||
	    pthread_create(&th_sv, NULL, threadfn_server, &th_sva) != 0) {
		fprintf(stderr, "Couldn't create threads\n");
		goto cleanup;
	}

	printf("Welcome\n"
	       "Enter '%s' or '%s' to see all available commands\n",
	       CMD_HELP, CMD_HELP_SHORT);

	while (active) {
		printf("> ");
		getline(&input, &input_len, stdin);
		for (i = 0; i < input_len; i++) {
			if ('\n' == input[i]) {
				input[i] = '\0';
				input_len = i + 1;
				break;
			}
		}

		if (input_len <= 1) {
			/* empty, do nothing */
		} else if (strcmp(input, CMD_HELP) == 0 ||
		    strcmp(input, CMD_HELP_SHORT) == 0) {
			printf(APP_HELP_COMMANDS);
		} else if (strcmp(input, CMD_QUIT) == 0 ||
		           strcmp(input, CMD_QUIT_SHORT) == 0) {
			active = false;
		} else {
			printf("Command not recognized\n");
		}

		free(input);
		input = NULL;
	}

	pthread_join(th_sv, NULL);
	pthread_join(th_w, NULL);

cleanup:
	Server_free(&sv);
	World_free(&w);

	return 0;
}
