/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2025  Andy Frank Schoknecht
 */

#include <core/core.h>
#include <extra/extra.h>
#include <gtk-3.0/gtk/gtk.h>
#include <stdio.h>

#include "embedded_glade.h"

#define BRUSH_RADIUS   2
#define SPAWNER_R      255
#define SPAWNER_G      0
#define SPAWNER_B      255
#define SPAWNER_A      128
#define TEMPERATURE    293.15
#define TOOL_HOVER_R   175
#define TOOL_HOVER_G   255
#define TOOL_HOVER_B   175
#define TOOL_HOVER_A   SPAWNER_A
#define WORLD_SIM_RATE 24
#define WORLD_SCALE    10
#define WORLD_W        40
#define WORLD_H        30

struct AppData {
	int           active;
	struct World  world;
	GMutex        mutex;

	GdkDisplay   *display;
	GdkSeat      *seat;
	GdkDevice    *pointer;
	GtkWidget    *win;
	GtkWidget    *toollist;
	GtkWidget    *materiallist;
	GtkWidget    *worldbox;
};

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

void
tick(gpointer user_data)
{
	struct AppData *ad = user_data;
	float           lasttick = 0.0;
	float           now = 0.0;
	GTimer         *timer;

	timer = g_timer_new();

	while (1) {
		now = g_timer_elapsed(timer, NULL);
		if (now - lasttick > 1.0 / WORLD_SIM_RATE) {
			lasttick = now;

			g_mutex_lock(&ad->mutex);
			if (!ad->active) {
				break;
			}

			world_update(&ad->world, TEMPERATURE);
			world_sim(&ad->world);
			g_mutex_unlock(&ad->mutex);

			gtk_widget_queue_draw(ad->worldbox);
		}
	}

	g_thread_exit(NULL);
}

void
toollist_changed_cb(GtkComboBox *toollist,
                    gpointer     user_data)
{
	struct AppData *ad = user_data;
	gint cur_tool;
	int i;

	cur_tool = gtk_combo_box_get_active(toollist);

	switch (cur_tool) {
	case TOOL_BRUSH:
		gtk_combo_box_text_remove_all((GtkComboBoxText*) ad->materiallist);
		for (i = 1; i < MAT_COUNT; i += 1) {
			gtk_combo_box_text_append_text((GtkComboBoxText*) ad->materiallist,
				                       MAT_NAME[i]);
		}
		gtk_combo_box_set_active((GtkComboBox*) ad->materiallist, 0);
		break;

	case TOOL_SPAWNER:
		gtk_combo_box_text_remove_all((GtkComboBoxText*) ad->materiallist);
		for (i = 0; i < MAT_COUNT; i += 1) {
			gtk_combo_box_text_append_text((GtkComboBoxText*) ad->materiallist,
				                       MAT_NAME[i]);
		}
		gtk_combo_box_set_active((GtkComboBox*) ad->materiallist, 0);
		break;

	case TOOL_ERASER:
		/* fallthrough */
	case TOOL_HEATER:
		/* fallthrough */
	case TOOL_COOLER:
		gtk_combo_box_text_remove_all((GtkComboBoxText*) ad->materiallist);
		break;
	}
}

gboolean
worldbox_draw_cb(GtkWidget *worldbox,
                 cairo_t   *cr,
                 gpointer   user_data)
{
	struct AppData *ad = user_data;
	struct Rgba glowc;
	gint mx, my;
	float r, g, b, a;
	GtkAllocation worldbox_rect;
	int x, y;

	for (x = 0; x < WORLD_W; x += 1) {
		for (y = 0; y < WORLD_H; y += 1) {
			r = color_int8_to_float(MAT_R[ad->world.dots[x][y]]);
			g = color_int8_to_float(MAT_G[ad->world.dots[x][y]]);
			b = color_int8_to_float(MAT_B[ad->world.dots[x][y]]);
			a = 1.0;
			cairo_set_source_rgba(cr, r, g, b, a);
			cairo_rectangle(cr,
			                x * WORLD_SCALE,
			                y * WORLD_SCALE,
			                WORLD_SCALE,
			                WORLD_SCALE);
			cairo_fill(cr);

			if (MAT_NONE == ad->world.dots[x][y]) {
				continue;
			}

			glowc = thermo_to_color(ad->world.thermo[x][y]);
			r = color_int8_to_float(glowc.r);
			g = color_int8_to_float(glowc.g);
			b = color_int8_to_float(glowc.b);
			a = color_int8_to_float(glowc.a);
			cairo_set_source_rgba(cr, r, g, b, a);
			cairo_rectangle(cr,
			                x * WORLD_SCALE,
			                y * WORLD_SCALE,
			                WORLD_SCALE,
			                WORLD_SCALE);
			cairo_fill(cr);
		}
	}

	gdk_window_get_device_position(gtk_widget_get_window(ad->win),
	                               ad->pointer,
	                               &mx,
	                               &my,
	                               NULL);
	gtk_widget_get_allocation(ad->worldbox, &worldbox_rect);

	r = color_int8_to_float(TOOL_HOVER_R);
	g = color_int8_to_float(TOOL_HOVER_G);
	b = color_int8_to_float(TOOL_HOVER_B);
	a = color_int8_to_float(TOOL_HOVER_A);
	cairo_set_source_rgba(cr, r, g, b, a);
	cairo_rectangle(cr,
	                mx - worldbox_rect.x - (BRUSH_RADIUS * WORLD_SCALE / 2),
	                my - worldbox_rect.y - (BRUSH_RADIUS * WORLD_SCALE / 2),
	                BRUSH_RADIUS * WORLD_SCALE,
	                BRUSH_RADIUS * WORLD_SCALE);
	cairo_fill(cr);

	return 0;
}

GtkWidget*
get_widget(GtkBuilder *builder, char *name)
{
	GtkWidget *ret;

	ret = GTK_WIDGET(gtk_builder_get_object(builder, name));
	if (NULL == ret) {
		printf("No %s in glade file\n", name);
		exit(1);
	}

	return ret;
}

int
main(int argc,
     char **argv)
{
	struct AppData  ad;
	GtkBuilder     *builder;
	long unsigned   i;
	GThread        *worldloop;
	struct World    world;

	gtk_init(&argc, &argv);
	extra_init();

	builder = gtk_builder_new_from_string((char*) EMBEDDED_GLADE,
	                                      EMBEDDED_GLADE_len - 1);
	if (NULL == builder) {
		printf("Could not make builder\n");
		exit(1);
	}

	ad.display = gdk_display_get_default();
	if (NULL == ad.display) {
		printf("Could not get display\n");
		exit(1);
	}

	ad.seat = gdk_display_get_default_seat(ad.display);
	ad.pointer = gdk_seat_get_pointer(ad.seat);
	if (NULL == ad.pointer) {
		printf("Could not get pointer\n");
		exit(1);
	}

	ad.win = get_widget(builder, "main");
	ad.toollist = get_widget(builder, "toollist");
	ad.materiallist = get_widget(builder, "materiallist");
	ad.worldbox = get_widget(builder, "worldbox");

	g_signal_connect((GObject*) ad.toollist,
	                 "changed",
	                 (GCallback) toollist_changed_cb,
	                 &ad);
	g_signal_connect((GObject*) ad.win,
	                 "destroy",
	                 gtk_main_quit,
	                 NULL);
	g_signal_connect((GObject*) ad.worldbox,
	                 "draw",
	                 (GCallback) worldbox_draw_cb,
	                 &ad);
	worldloop = g_thread_new("worldloop", (GThreadFunc) tick, &ad);

	ad.active = 1;
	g_mutex_init(&ad.mutex);

	for (i = 0; i < TOOL_COUNT; i += 1) {
		gtk_combo_box_text_append_text((GtkComboBoxText*) ad.toollist,
		                               TOOL_NAME[i]);
	}

	gtk_combo_box_set_active((GtkComboBox*) ad.toollist, TOOL_BRUSH);
	toollist_changed_cb((GtkComboBox*) ad.toollist, &ad);

	ad.world = world_new(WORLD_W, WORLD_H, TEMPERATURE);
	set_worldbox_size(ad.worldbox,
	                  WORLD_W * WORLD_SCALE,
	                  WORLD_H * WORLD_SCALE);

	ad.world.dots[WORLD_W / 2 - 1][0] = MAT_SAND;
	ad.world.dots  [WORLD_W / 2][0] = MAT_SAND;
	ad.world.thermo[WORLD_W / 2][0] = 7000;
	ad.world.dots[WORLD_W / 2 + 1][1] = MAT_WATER;
	ad.world.dots[WORLD_W / 2][1] = MAT_WATER;

	gtk_widget_show_all(ad.win);
	gtk_main();

	g_mutex_lock(&ad.mutex);
	ad.active = 0;
	g_mutex_unlock(&ad.mutex);

	g_thread_join(worldloop);
	world_free(&ad.world);

	return 0;
}
