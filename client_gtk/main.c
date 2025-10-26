/* SPDX-License-Identifier: LGPL-2.1-only
 * Copyright (C) 2024 - 2025  Andy Frank Schoknecht
 */

#include <core/core.h>
#include <extra/extra.h>
#include <gtk-3.0/gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#include "embedded_glade.h"

#define BUFSIZE 128

#define STD_BRUSH_RADIUS   2
#define STD_ERASER_RADIUS  5
#define STD_THERMO_RADIUS  STD_BRUSH_RADIUS

#define STD_TEMPERATURE    293.15
#define STD_WORLD_SIM_RATE 24

#define THERMOTOOL_RATE 100

#define MAX_WORLD_SIM_RATE (STD_WORLD_SIM_RATE * 2)
#define MIN_WORLD_SIM_RATE 0

#define WORLD_SCALE    10
#define WORLD_W        40
#define WORLD_H        30

#define SPAWNER_R      255
#define SPAWNER_G      0
#define SPAWNER_B      255
#define SPAWNER_A      128

#define TOOL_HOVER_R   175
#define TOOL_HOVER_G   255
#define TOOL_HOVER_B   175
#define TOOL_HOVER_A   SPAWNER_A

struct AppData {
	int           active;
	enum Mat      cur_mat;
	enum Tool     cur_tool;
	int           brush_radius;
	int           eraser_radius;
	int           thermo_radius;
	struct World  world;
	GMutex        mutex;

	GdkDisplay   *display;
	GdkSeat      *seat;
	GdkDevice    *pointer;
	GtkWidget    *win;
	GtkWidget    *simspeed;
	GtkWidget    *spawnertemperature;
	GtkWidget    *temperature;
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
get_mouse_world_coords(struct AppData *ad,
                       float          *x,
                       float          *y)
{
	int            mx = 0;
	int            my = 0;
	GdkWindow     *win;
	GtkAllocation  worldbox_rect;

	win = gtk_widget_get_window(ad->win);
	gdk_window_get_device_position(win, ad->pointer, &mx, &my, NULL);
	gtk_widget_get_allocation(ad->worldbox, &worldbox_rect);

	*x = ((float) (mx - worldbox_rect.x) / (float) WORLD_SCALE);
	*y = ((float) (my - worldbox_rect.y) / (float) WORLD_SCALE);
}

gpointer
worldloop(gpointer user_data)
{
	struct AppData  *ad = user_data;
	GdkModifierType  mouse_state;
	float            mx;
	float            my;
	float            lasttick = 0.0;
	float            lastthermo = 0.0;
	float            now = 0.0;
	GTimer          *timer;
	int              x;
	int              y;

	timer = g_timer_new();

	while (1) {
		now = g_timer_elapsed(timer, NULL);

		get_mouse_world_coords(ad, &mx, &my);
		x = mx;
		y = my;
		gdk_device_get_state(ad->pointer,
		                     gtk_widget_get_window(ad->win),
		                     NULL,
		                     &mouse_state);

		if (mouse_state & GDK_BUTTON1_MASK &&
		    x >= 0 &&
		    y >= 0 &&
		    x < WORLD_W &&
		    y < WORLD_H) {
			switch (ad->cur_tool) {
			case TOOL_BRUSH:
				world_use_brush(&ad->world,
						ad->cur_mat,
						STD_TEMPERATURE,
						x, y,
						ad->brush_radius);
				break;

			case TOOL_SPAWNER:
				ad->world.spawner[x][y] = 1;
				ad->world.spawner_mat[x][y] = ad->cur_mat;
				break;

			case TOOL_ERASER:
				world_use_eraser(&ad->world, x, y, ad->eraser_radius);
				break;

			case TOOL_COOLER:
				if (1.0 / THERMOTOOL_RATE < now - lastthermo) {
					world_use_cooler(&ad->world,
							 1.0,
							 x, y,
							 ad->thermo_radius);
					lastthermo = now;
				}
				break;

			case TOOL_HEATER:
				if (1.0 / THERMOTOOL_RATE < now - lastthermo) {
					world_use_heater(&ad->world,
							 1.0,
							 x, y,
							 ad->thermo_radius);
					lastthermo = now;
				}
				break;

			default:
				break;
			}
		}

		if (now - lasttick > 1.0 / STD_WORLD_SIM_RATE) {
			lasttick = now;

			g_mutex_lock(&ad->mutex);
			if (!ad->active) {
				break;
			}

			world_update(&ad->world, STD_TEMPERATURE);
			world_sim(&ad->world);
			g_mutex_unlock(&ad->mutex);

			gtk_widget_queue_draw(ad->worldbox);
		}
	}

	g_thread_exit(NULL);

	return NULL;
}

void
update_materiallist(gpointer user_data)
{
	struct AppData *ad = user_data;
	int i;
	int spwn_tmprtr;

	switch (ad->cur_tool) {
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
		gtk_combo_box_text_append_text((GtkComboBoxText*) ad->materiallist,
		                       MAT_NAME[MAT_NONE]);
		for (i = 1; i < MAT_COUNT; i += 1) {
			spwn_tmprtr = atof(gtk_entry_get_text((GtkEntry*) ad->spawnertemperature));
			if (MS_STATIC == mat_thermo_to_state(i, spwn_tmprtr))
				continue;

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

	default:
		break;
	}
}

/* Additionally, there are known bugs in GTK3
 * where the delta_x and delta_y fields in the GdkEventScroll structure
 * are consistently zero,
 * rendering them unusable for determining scroll direction.
 * This issue has persisted even in newer GTK versions, including GTK4,
 * making it impossible to reliably detect scroll events using these fields.
 * As a workaround, developers have been advised to use
 * the GDK_SCROLL_UP and GDK_SCROLL_DOWN event types
 * instead of relying on the delta values
 * - Leo (Brave Search AI)
 *
 * I am filled with tremendous amounts of piss...
 * BUT WAIT, THERE IS MORE!
 * Apparently https://docs.gtk.org/gtk3/signal.Widget.scroll-event.html
 * lists the event parameter as type GdkEventScroll.
 * This is putrid garbage trash, as it is actually a pointer of that.
 * If you try to follow the documentation, you're fucked.
 * PLUS none of the deltas will work,
 * if you haven't added GDK_SMOOTH_SCROLL_MASK, while the docs say
 * that you just need GDK_SCROLL_MASK.
 */
gboolean
worldbox_scroll_event_cb(GtkWidget      *dummy,
                         GdkEventScroll *event,
                         gpointer        user_data)
{
	struct AppData *ad = user_data;
	int delta = 0;

	(void) dummy;

	delta = (int) event->delta_y * -1;

	switch (ad->cur_tool) {
	case TOOL_BRUSH:
		ad->brush_radius += delta;
		if (ad->brush_radius < 0)
			ad->brush_radius = 0;
		break;

	case TOOL_SPAWNER:
		break;

	case TOOL_ERASER:
		ad->eraser_radius += delta;
		if (ad->eraser_radius < 0)
			ad->eraser_radius = 0;
		break;

	case TOOL_HEATER:
		/* fallthrough */
	case TOOL_COOLER:
		ad->thermo_radius += delta;
		if (ad->thermo_radius < 0)
			ad->thermo_radius = 0;
		break;

	default:
		break;
	}

	return 1;
}

void
spawnertemperature_changed_cb(void     *dummy,
                              gpointer  user_data)
{
	(void) dummy;

	update_materiallist(user_data);
}

void
materiallist_changed_cb(GtkComboBoxText *materiallist,
                        gpointer         user_data)
{
	struct AppData *ad = user_data;
	enum Mat i;
	gchar *sel_text;

	sel_text = gtk_combo_box_text_get_active_text(materiallist);

	if (NULL == sel_text) {
		ad->cur_mat = MAT_NONE;
		return;
	}

	for (i = 0; i < MAT_COUNT; i += 1) {
		if (strcmp(MAT_NAME[i], sel_text) == 0) {
			ad->cur_mat = i;
		}
	}

	g_free(sel_text);
}

void
toollist_changed_cb(GtkComboBox *toollist,
                    gpointer     user_data)
{
	struct AppData *ad = user_data;

	ad->cur_tool = gtk_combo_box_get_active(toollist);

	update_materiallist(user_data);
}

gboolean
worldbox_draw_cb(void     *dummy,
                 cairo_t  *cr,
                 gpointer  user_data)
{
	struct AppData *ad = user_data;
	GdkCursor      *cursor = NULL;
	struct Rgba     glowc;
	float           mx, my;
	float           r, g, b, a;
	int             tool_radius = 0;
	int             tooldraw_size, tooldraw_x, tooldraw_y;
	GdkWindow      *win;
	int             x, y;

	(void) dummy;

	switch (ad->cur_tool) {
	case TOOL_BRUSH:
		tool_radius = ad->brush_radius;
		break;

	case TOOL_SPAWNER:
		tool_radius = 0;
		break;

	case TOOL_ERASER:
		tool_radius = ad->eraser_radius;
		break;

	case TOOL_HEATER:
		/* fallthrough */
	case TOOL_COOLER:
		tool_radius = ad->thermo_radius;
		break;

	default:
		break;
	}

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

	win = gtk_widget_get_window(ad->win);
	get_mouse_world_coords(ad, &mx, &my);

	if (mx >= 0.0 &&
	    mx < (float) WORLD_W &&
	    my >= 0.0 &&
	    my < (float) WORLD_H) {
		cursor = gdk_cursor_new(GDK_BLANK_CURSOR);

		tooldraw_size = (tool_radius * 2 + 1) * WORLD_SCALE;
		tooldraw_x = ((int) mx * WORLD_SCALE) - (tool_radius * WORLD_SCALE);
		tooldraw_y = ((int) my * WORLD_SCALE) - (tool_radius * WORLD_SCALE);

		r = color_int8_to_float(TOOL_HOVER_R);
		g = color_int8_to_float(TOOL_HOVER_G);
		b = color_int8_to_float(TOOL_HOVER_B);
		a = color_int8_to_float(TOOL_HOVER_A);
		cairo_set_source_rgba(cr, r, g, b, a);
		cairo_rectangle(cr, tooldraw_x, tooldraw_y, tooldraw_size, tooldraw_size);
		cairo_fill(cr);
	}

	gdk_window_set_cursor(win, cursor);

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
	char            buf[BUFSIZE];
	long unsigned   i;
	GThread        *worldloop_thread;
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
	ad.simspeed = get_widget(builder, "simspeed");
	ad.spawnertemperature = get_widget(builder, "spawnertemperature");
	ad.temperature = get_widget(builder, "temperature");
	ad.toollist = get_widget(builder, "toollist");
	ad.materiallist = get_widget(builder, "materiallist");
	ad.worldbox = get_widget(builder, "worldbox");

	gtk_widget_add_events(ad.worldbox,
	                      GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK);

	g_signal_connect((GObject*) ad.materiallist,
	                 "changed",
	                 (GCallback) materiallist_changed_cb,
	                 &ad);
	g_signal_connect((GObject*) ad.spawnertemperature,
	                 "changed",
	                 (GCallback) spawnertemperature_changed_cb,
	                 &ad);
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
	g_signal_connect((GObject*) ad.worldbox,
	                 "scroll-event",
	                 (GCallback) worldbox_scroll_event_cb,
	                 &ad);
	worldloop_thread = g_thread_new("worldloop", worldloop, &ad);

	ad.active = 1;
	ad.brush_radius = STD_BRUSH_RADIUS;
	ad.eraser_radius = STD_ERASER_RADIUS;
	ad.thermo_radius = STD_THERMO_RADIUS;
	g_mutex_init(&ad.mutex);

	for (i = 0; i < TOOL_COUNT; i += 1) {
		gtk_combo_box_text_append_text((GtkComboBoxText*) ad.toollist,
		                               TOOL_NAME[i]);
	}

	gtk_combo_box_set_active((GtkComboBox*) ad.toollist, TOOL_BRUSH);
	update_materiallist(&ad);
	gtk_range_set_range((GtkRange*) ad.simspeed,
	                    MIN_WORLD_SIM_RATE,
	                    MAX_WORLD_SIM_RATE);
	gtk_range_set_value((GtkRange*) ad.simspeed, STD_WORLD_SIM_RATE);
	snprintf(buf, BUFSIZE, "%f", STD_TEMPERATURE);
	gtk_entry_set_text((GtkEntry*) ad.spawnertemperature, buf);
	snprintf(buf, BUFSIZE, "%f", STD_TEMPERATURE);
	gtk_entry_set_text((GtkEntry*) ad.temperature, buf);

	ad.world = world_new(WORLD_W, WORLD_H, STD_TEMPERATURE);
	set_worldbox_size(ad.worldbox,
	                  WORLD_W * WORLD_SCALE,
	                  WORLD_H * WORLD_SCALE);

	gtk_widget_show_all(ad.win);
	gtk_main();

	g_mutex_lock(&ad.mutex);
	ad.active = 0;
	g_mutex_unlock(&ad.mutex);

	g_thread_join(worldloop_thread);
	world_free(&ad.world);

	return 0;
}
