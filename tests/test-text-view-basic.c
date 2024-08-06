/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
