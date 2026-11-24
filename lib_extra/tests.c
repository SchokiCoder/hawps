/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2024 - 2026  Andy Frank Schoknecht
 */

#define HAWPS_IMPL
#include <assert.h>
#include <hawps_core.h>
#include <hawps_extra.h>
#include <pthread.h>
#include <stdio.h>

#define SLOTS             8
#define TICKRATE          24.0
#define PORT              25565
#define PORT_STR          "25565"
#define WORLD_W           10
#define WORLD_H           10
#define WORLD_TEMPERATURE 273.15 + 20

struct ThArgsServer {
	volatile bool *active;
	struct Server *sv;
	struct World  *w;
};

static struct World world;

void*
threadfn_sv(void *arg)
{
	struct ThArgsServer *args = arg;
	NSecs                last_tick = 0;
	NSecs                now = 0;

	while (*args->active) {
		now = get_now();
		if (now - last_tick < (NSecs) (NANOS_PER_SEC / TICKRATE)) {
			continue;
		}
		last_tick = now;

		Server_tick(args->sv, now, args->w);
	}

	return NULL;
}

void
test_rgba_blend(void)
{
	const struct Rgba src = {
		.r = 30,
		.g = 40,
		.b = 50,
		.a = 100,
	};
	const struct Rgba dest = {
		.r = 100,
		.g = 125,
		.b = 75,
		.a = 255,
	};
	const struct Rgba expected = {
		.r = 72,
		.g = 91,
		.b = 64,
		.a = 255,
	};
	struct Rgba res;

	res = Rgba_blend(src, dest);

	assert(expected.r == res.r);
	assert(expected.g == res.g);
	assert(expected.b == res.b);
	assert(expected.a == res.a);
}

void
test_heat_glow(void)
{
	struct Rgba cold;
	struct Rgba draper;
	struct Rgba hot;
	struct Rgba super_hot;

	cold = Rgba_from_thermo(0.0);
	draper = Rgba_from_thermo(800.0);
	hot = Rgba_from_thermo(3000.0);
	super_hot = Rgba_from_thermo(9001.0);

	assert(0 == cold.a);
	assert(draper.a > 0);
	assert(draper.a < 64);
	assert(hot.a > 128);
	assert(hot.a < 256);
	assert(0 == super_hot.a);
}

void
test_strtoaddr(void)
{
	char *host = NULL;
	char *port = NULL;

	strtoaddr("192.168.0.43", &host, &port);
	assert(strcmp("192.168.0.43", host) == 0);
	assert(NULL == port);
	free(host);
	host = NULL;

	strtoaddr("102.16.0.200:25565", &host, &port);
	assert(strcmp("102.16.0.200", host) == 0);
	assert(strcmp("25565", port) == 0);
	free(host);
	host = NULL;
	free(port);
	port = NULL;

	strtoaddr("a00:d993::", &host, &port);
	assert(strcmp("a00:d993::", host) == 0);
	assert(NULL == port);
	free(host);
	host = NULL;

	strtoaddr("[a00:d993::]", &host, &port);
	assert(strcmp("a00:d993::", host) == 0);
	assert(NULL == port);
	free(host);
	host = NULL;

	strtoaddr("[a00:d993::]:25565", &host, &port);
	assert(strcmp("a00:d993::", host) == 0);
	assert(strcmp("25565", port) == 0);
	free(host);
	host = NULL;
	free(port);
	port = NULL;

	strtoaddr("www.youactuallyhostthis.net", &host, &port);
	assert(strcmp("www.youactuallyhostthis.net", host) == 0);
	assert(NULL == port);
	free(host);
	host = NULL;

	strtoaddr("www.youactuallyhostthis.net:420", &host, &port);
	assert(strcmp("www.youactuallyhostthis.net", host) == 0);
	assert(strcmp("420", port) == 0);
	free(host);
	host = NULL;
	free(port);
	port = NULL;

	strtoaddr("]a00:d993::[:25565", &host, &port);
	assert(NULL == host);
	assert(NULL == port);
}

void
test_connect(void)
{
	bool                active = true;
	struct Client       cl;
	struct Server       sv;
	pthread_t           th_sv = 0;
	struct ThArgsServer tha_sv;
	struct World        w_cl;
	struct World        w_sv;

	memset(&w_cl, 0, sizeof(w_cl));
	w_sv = World_new(WORLD_W, WORLD_H);

	assert(Server_new(&sv, SLOTS, PORT, WORLD_W, WORLD_H) == NE_OKAY);

	tha_sv.active = &active;
	tha_sv.sv = &sv;
	tha_sv.w = &w_sv;

	assert(pthread_create(&th_sv, NULL, threadfn_sv, &tha_sv) == 0);

	assert(Client_connect(&cl, "localhost", PORT_STR, &w_cl) == NE_OKAY);
	assert(w_cl.w == WORLD_W);
	assert(w_cl.h == WORLD_H);

	active = false;
	pthread_join(th_sv, NULL);

	Client_free(&cl);
	Server_free(&sv);
	World_free(&w_cl);
	World_free(&w_sv);
}

int
main()
{
	hawps_core_init();
	hawps_extra_init();

	test_rgba_blend();
	test_heat_glow();
	test_strtoaddr();
	test_connect();

	printf("All tests passed :)\n");
	return 0;
}
