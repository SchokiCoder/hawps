/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2025  Andy Frank Schoknecht
 */

#include <core/core.h>
#include <extra/extra.h>
#include <stdlib.h>
#include <string.h>
#include <tcl.h>
#include <time.h>
#include <tk.h>

#define PIXELSIZE 4
#define R 0
#define G 1
#define B 2
#define A 3

#define TCL_ARGC          2
#define CELSIUS_TO_KELVIN 273.15

#define ALPHA_LOSS_PER_STATE 51

/* simulate every nth tick (sim_rate = tick_rate / sim_subsample) */
#define SIM_SUBSAMPLE 5
#define TICK_RATE     120

#define STD_TEMPERATURE (20.0 + CELSIUS_TO_KELVIN)

#define WORLD_SCALE 10
#define WORLD_W     40
#define WORLD_H     30

#define TOOL_HOVER_R 175
#define TOOL_HOVER_G 255
#define TOOL_HOVER_B 175
#define TOOL_HOVER_A 128

#define TOOL_RADIUS 2

unsigned char      ***pixels = NULL;
Tk_PhotoImageBlock    pixblock;
unsigned char        *pixbuf = NULL;
int                   pixbuf_size = 0;
int                   tcl_mouse_x = -1;
int                   tcl_mouse_y = -1;
int                   tcl_world_x = 9001;
int                   tcl_world_y = 9001;
struct World          world;
Tk_PhotoHandle        worldimg = NULL;

int
init_proc(Tcl_Interp *interp);

int
mainloop(ClientData data,
         Tcl_Interp *interp,
         int objc,
         Tcl_Obj *const objv[]);

int
motion(ClientData data,
       Tcl_Interp *interp,
       int objc,
       Tcl_Obj *const objv[]);

void
tick(ClientData data);

int
world_draw(Tcl_Interp *interp);

int
init_proc(Tcl_Interp *interp)
{
	Tcl_Namespace *ns;
	int ret;

	ret = Tcl_Init(interp);
	if (TCL_ERROR == ret) {
		fprintf(stderr, "Could not initialize Tcl\n");
		return ret;
	}

	ret = Tk_Init(interp);
	if (TCL_ERROR == ret) {
		fprintf(stderr, "Could not initialize TK\n");
		return ret;
	}

	ns = Tcl_CreateNamespace(interp, "hawps", NULL, NULL);
	if (NULL == ns) {
		fprintf(stderr, "Could not create a namespace\n");
		return TCL_ERROR;
	}

	Tcl_CreateObjCommand(interp, "hawps::mainloop", mainloop, NULL, NULL);
	Tcl_CreateObjCommand(interp, "hawps::motion", motion, NULL, NULL);

	return ret;
}

int
mainloop(ClientData data,
         Tcl_Interp *interp,
         int objc,
         Tcl_Obj *const objv[])
{
	float ticks_since_sim = 9001;
	float last_tick = -9001.0;
	float now;
	int x, y;

	(void) data;
	(void) objc;
	(void) objv;

	worldimg = Tk_FindPhoto(interp, "worldimg");
	if (NULL == worldimg) {
		fprintf(stderr, "Could not find worldimg\n");
	}

	pixbuf_size = WORLD_W * WORLD_H * PIXELSIZE;
	pixbuf = calloc(pixbuf_size, sizeof(unsigned char));
	if (NULL == pixbuf) {
		fprintf(stderr, "Could not allocate memory\n");
		return TCL_ERROR;
	}
	pixels = calloc(WORLD_W, sizeof(unsigned char**));
	for (x = 0; x < WORLD_W; x++) {
		pixels[x] = calloc(WORLD_H, sizeof(unsigned char*));
		for (y = 0; y < WORLD_H; y++) {
			pixels[x][y] = &pixbuf[((y * WORLD_W) + x) * PIXELSIZE];
		}
	}

	pixblock.pixelPtr = pixbuf;
	pixblock.width = WORLD_W;
	pixblock.height = WORLD_H;
	pixblock.pitch = WORLD_W * PIXELSIZE;
	pixblock.pixelSize = PIXELSIZE;
	pixblock.offset[0] = R;
	pixblock.offset[1] = G;
	pixblock.offset[2] = B;
	pixblock.offset[3] = A;

	extra_init();

	// TODO add end conditon or break
	while (1) {
		now = (float) clock() / (float) CLOCKS_PER_SEC;
		if ((now - last_tick) > (1.0 / TICK_RATE)) {
			last_tick = now;
			ticks_since_sim += 1;
			if (ticks_since_sim >= SIM_SUBSAMPLE) {
				ticks_since_sim = 0;

				Tcl_DoWhenIdle(tick, interp);
			}

			Tcl_DoOneEvent(TCL_ALL_EVENTS | TCL_DONT_WAIT);
		}
	}

	return TCL_OK;
}

int
motion(ClientData data,
       Tcl_Interp *interp,
       int objc,
       Tcl_Obj *const objv[])
{
	(void) data;
	(void) objc;

	Tcl_GetIntFromObj(interp, objv[1], &tcl_world_x);
	Tcl_GetIntFromObj(interp, objv[2], &tcl_world_y);
	Tcl_GetIntFromObj(interp, objv[3], &tcl_mouse_x);
	Tcl_GetIntFromObj(interp, objv[4], &tcl_mouse_y);

	return TCL_OK;
}


void
tick(ClientData data)
{
	Tcl_Interp *interp = data;

	world_update(&world, STD_TEMPERATURE);
	world_sim(&world);
	world_draw(interp);
}

int
world_draw(Tcl_Interp *interp)
{
	struct Rgba col;
	int x, y;
	int x1, y1;
	int x2, y2;

	/* draw bg */
	for (x = 0; x < WORLD_W; x += 1) {
		for (y = 0; y < WORLD_H; y += 1) {
			pixels[x][y][R] = 0;
			pixels[x][y][G] = 0;
			pixels[x][y][B] = 0;
			pixels[x][y][A] = 255;

		}
	}
	Tk_PhotoPutZoomedBlock(interp, worldimg, &pixblock,
	                       0, 0,
	                       WORLD_W * WORLD_SCALE, WORLD_H * WORLD_SCALE,
	                       WORLD_SCALE, WORLD_SCALE,
	                       1, 1,
	                       TK_PHOTO_COMPOSITE_SET);

	/* draw dots */
	for (x = 0; x < WORLD_W; x += 1) {
		for (y = 0; y < WORLD_H; y += 1) {
			switch (world.states[x][y]) {
			case MS_LIQUID:
				col.a = 255 - ALPHA_LOSS_PER_STATE;
				break;

			case MS_GAS:
				col.a = 255 - (ALPHA_LOSS_PER_STATE * 2);
				break;

			default:
				col.a = 255;
				break;
			}

			pixels[x][y][R] = MAT_R[world.dots[x][y]];
			pixels[x][y][G] = MAT_G[world.dots[x][y]];
			pixels[x][y][B] = MAT_B[world.dots[x][y]];
			pixels[x][y][A] = col.a;
		}
	}
	Tk_PhotoPutZoomedBlock(interp, worldimg, &pixblock,
	                       0, 0,
	                       WORLD_W * WORLD_SCALE, WORLD_H * WORLD_SCALE,
	                       WORLD_SCALE, WORLD_SCALE,
	                       1, 1,
	                       TK_PHOTO_COMPOSITE_OVERLAY);

	/* draw dotglow */
	for (x = 0; x < WORLD_W; x += 1) {
		for (y = 0; y < WORLD_H; y += 1) {
			col = thermo_to_color(world.thermo[x][y]);
			pixels[x][y][R] = col.r;
			pixels[x][y][G] = col.g;
			pixels[x][y][B] = col.b;
			pixels[x][y][A] = col.a;

		}
	}
	Tk_PhotoPutZoomedBlock(interp, worldimg, &pixblock,
	                       0, 0,
	                       WORLD_W * WORLD_SCALE, WORLD_H * WORLD_SCALE,
	                       WORLD_SCALE, WORLD_SCALE,
	                       1, 1,
	                       TK_PHOTO_COMPOSITE_OVERLAY);

	/* draw tool hover */
	x = ((tcl_mouse_x - tcl_world_x) / WORLD_SCALE);
	y = ((tcl_mouse_y - tcl_world_y) / WORLD_SCALE);

	x1 = x - TOOL_RADIUS;
	if (x1 < 0)
		x1 = 0;

	y1 = y - TOOL_RADIUS;
	if (y1 < 0)
		y1 = 0;

	x2 = x + TOOL_RADIUS + 1;
	if (x2 > WORLD_W)
		x2 = WORLD_W;

	y2 = y + TOOL_RADIUS + 1;
	if (y2 > WORLD_H)
		y2 = WORLD_H;

	memset(pixbuf, 0, pixbuf_size);
	for (x = x1; x < x2; x += 1) {
		for (y = y1; y < y2; y += 1) {
			pixels[x][y][R] = TOOL_HOVER_R;
			pixels[x][y][G] = TOOL_HOVER_G;
			pixels[x][y][B] = TOOL_HOVER_B;
			pixels[x][y][A] = TOOL_HOVER_A;

		}
	}
	Tk_PhotoPutZoomedBlock(interp, worldimg, &pixblock,
	                       0, 0,
	                       WORLD_W * WORLD_SCALE, WORLD_H * WORLD_SCALE,
	                       WORLD_SCALE, WORLD_SCALE,
	                       1, 1,
	                       TK_PHOTO_COMPOSITE_OVERLAY);

	return TCL_OK;
}

int
main(int argc,
     char **argv)
{
	int i;
	char *tcl_args[TCL_ARGC];

	for (i = 0; i < TCL_ARGC; i += 1) {
		tcl_args[i] = malloc(sizeof(char) * 64);
	}

	strcpy(tcl_args[0], "APP_NAME");
	strcpy(tcl_args[1], "./client_tk/main.tcl");

	world = world_new(WORLD_W, WORLD_H, STD_TEMPERATURE);
	world.dots[0][2] = MAT_WATER;
	world.thermo[0][2] = CELSIUS_TO_KELVIN - 10;
	world.dots[2][0] = MAT_WATER;
	world.dots[2][2] = MAT_WATER;
	world.thermo[2][2] = CELSIUS_TO_KELVIN + 3000;
	world.dots[4][2] = MAT_WATER;
	world.dots[2][4] = MAT_WATER;

	Tcl_Main(2, tcl_args, init_proc);

	for (i = 0; i < WORLD_W; i += 1) {
		free(pixels[i]);
	}
	free(pixels);
	free(pixbuf);
	for (i = 0; i < TCL_ARGC; i += 1) {
		free(tcl_args[i]);
	}

	world_free(&world);
}
