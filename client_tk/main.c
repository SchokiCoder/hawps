/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2025  Andy Frank Schoknecht
 */

#include <core/core.h>
#include <stdlib.h>
#include <string.h>
#include <tcl.h>
#include <time.h>
#include <tk.h>

#define R 0
#define G 1
#define B 2
#define A 3

#define TCL_ARGC          2
#define CELSIUS_TO_KELVIN 273.15
#define PIXELSIZE         4

#define SIM_RATE 24
#define STD_TEMPERATURE    (20.0 + CELSIUS_TO_KELVIN)

#define ALPHA_LOSS_PER_STATE 51

#define WORLD_SCALE 10
#define WORLD_W     40
#define WORLD_H     30

unsigned char      ***pixels;
Tk_PhotoImageBlock    pixblock;
unsigned char        *pixbuf = NULL;
int                   pixbuf_size = 0;
struct World          world;
Tk_PhotoHandle        worldimg = NULL;

int
init_proc(Tcl_Interp *interp);

int
mainloop(ClientData data,
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

	return ret;
}

int
mainloop(ClientData data,
         Tcl_Interp *interp,
         int objc,
         Tcl_Obj *const objv[])
{
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

	while (1) {
		now = (float) clock() / (float) CLOCKS_PER_SEC;
		if ((now - last_tick) > (1.0 / SIM_RATE)) {
			last_tick = now;

			Tcl_DoWhenIdle(tick, interp);
		}

		Tcl_DoOneEvent(TCL_ALL_EVENTS | TCL_DONT_WAIT);
	}


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
	int a;
	int x, y;

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
				a = 255 - ALPHA_LOSS_PER_STATE;
				break;

			case MS_GAS:
				a = 255 - (ALPHA_LOSS_PER_STATE * 2);
				break;

			default:
				a = 255;
				break;
			}

			pixels[x][y][R] = MAT_R[world.dots[x][y]];
			pixels[x][y][G] = MAT_G[world.dots[x][y]];
			pixels[x][y][B] = MAT_B[world.dots[x][y]];
			pixels[x][y][A] = a;
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
	world.thermo[2][2] = CELSIUS_TO_KELVIN + 300;
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
