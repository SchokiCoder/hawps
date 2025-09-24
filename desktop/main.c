/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2025  Andy Frank Schoknecht
 */

#include <core/mat.h>
#include <gtk-3.0/gtk/gtk.h>
#include <stdio.h>

#include "embedded_glade.h"

#define TEMPERATURE 293.15
#define WORLD_SCALE 10
#define WORLD_W     40
#define WORLD_H     30

void
set_worldbox_size(GtkWidget *worldbox,
                  const int  w,
                  const int  h)
{
	GValue gw = G_VALUE_INIT;
	GValue gh = G_VALUE_INIT;

	g_value_init(&gw, G_TYPE_INT);
	g_value_init(&gh, G_TYPE_INT);

	g_value_set_int(&gw, w);
	g_value_set_int(&gh, h);

	g_object_set_property((GObject*) worldbox, "width-request", &gw);
	g_object_set_property((GObject*) worldbox, "height-request", &gh);
	gtk_widget_queue_resize_no_redraw(worldbox);

	g_value_unset(&gw);
	g_value_unset(&gh);
}

gboolean
worldbox_draw_cb(GtkWidget *worldbox,
                 cairo_t   *cr,
                 gpointer   user_data)
{
	struct World *world = user_data;
	int x, y;

	for (x = 0; x < WORLD_W; x += 1) {
		for (y = 0; y < WORLD_H; y += 1) {
			cairo_set_source_rgba(cr,
			                      MAT_R[world->dots[x][y]] / 255,
			                      MAT_G[world->dots[x][y]] / 255,
			                      MAT_B[world->dots[x][y]] / 255,
			                      1.0);
			cairo_rectangle(cr,
			                x * WORLD_SCALE,
			                y * WORLD_SCALE,
			                WORLD_SCALE,
			                WORLD_SCALE);
			cairo_fill(cr);
		}
	}

	return 0;
}

int
main(int argc,
     char **argv)
{
	GtkBuilder   *builder;
	GtkWidget    *win;
	GtkWidget    *worldbox;
	struct World  world;

	gtk_init(&argc, &argv);

	builder = gtk_builder_new_from_string((char*) EMBEDDED_GLADE,
	                                      EMBEDDED_GLADE_len - 1);
	if (NULL == builder) {
		printf("Could not make builder\n");
		exit(1);
	}

	win = GTK_WIDGET(gtk_builder_get_object(builder, "main"));
	if (NULL == win) {
		printf("No window in glade file\n");
		exit(1);
	}

	worldbox = GTK_WIDGET(gtk_builder_get_object(builder, "worldbox"));
	if (NULL == worldbox) {
		printf("No worldbox in glade file\n");
		exit(1);
	}

	g_signal_connect((GObject*) win,
	                 "destroy",
	                 gtk_main_quit,
	                 NULL);
	g_signal_connect((GObject*) worldbox,
	                 "draw",
	                 (GCallback) worldbox_draw_cb,
	                 &world);

	world = world_new(WORLD_W, WORLD_H, TEMPERATURE);
	set_worldbox_size(worldbox,
	                  WORLD_W * WORLD_SCALE,
	                  WORLD_H * WORLD_SCALE);

	world.dots[WORLD_W / 2][0] = MAT_SAND;
	world.dots[WORLD_W / 2 + 1][1] = MAT_WATER;

	gtk_widget_show_all(win);
	gtk_main();

	world_free(&world);

	return 0;
}
