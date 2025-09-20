// SPDX-License-Identifier: LGPL-2.1-only
// Copyright (C) 2024 - 2025  Andy Frank Schoknecht

#include <gtk-3.0/gtk/gtk.h>
#include <stdio.h>

#include "embedded_glade.h"

int main(int argc, char **argv)
{
	GtkBuilder *builder;
	GtkWidget  *win;

	gtk_init(&argc, &argv);

	builder = gtk_builder_new_from_string(EMBEDDED_GLADE, EMBEDDED_GLADE_len - 1);
	if (NULL == builder) {
		printf("Could not make builder\n");
		exit(1);
	}

	win = GTK_WIDGET(gtk_builder_get_object(builder, "main"));
	if (NULL == win) {
		printf("No window in glade file\n");
		exit(1);
	}

	g_signal_connect(win, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	gtk_widget_show_all(win);
	gtk_main();

	return 0;
}
