/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <gspell/gspell.h>

gint
main (gint    argc,
      gchar **argv)
{
	GtkWidget *window;
	GtkWidget *scrolled_window;
	GtkTextView *gtk_view;
	GspellTextView *gspell_view;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size (GTK_WINDOW (window), 600, 400);
	g_signal_connect (window, "destroy", gtk_main_quit, NULL);

	gtk_view = GTK_TEXT_VIEW (gtk_text_view_new ());
	gspell_view = gspell_text_view_get_from_gtk_text_view (gtk_view);
	gspell_text_view_basic_setup (gspell_view);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	g_object_set (scrolled_window,
		      "expand", TRUE,
		      "margin", 6,
		      NULL);

	gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (gtk_view));
	gtk_container_add (GTK_CONTAINER (window), scrolled_window);
	gtk_widget_show_all (window);

	gtk_main ();

	return 0;
}

/* ex:set ts=8 noet: */
