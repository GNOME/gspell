/* SPDX-FileCopyrightText: 2015, 2017 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/*
 * Usage: gspell-app [lang_code]
 *
 * gspell-app is a small spell-checker. It does only one thing but does it
 * (hopefully) well.
 */

#include "gspell-config.h"
#include <locale.h>
#include <libintl.h>
#include <gspell/gspell.h>

#define GSPELL_TYPE_APP_CONTENT (gspell_app_content_get_type ())
G_DECLARE_FINAL_TYPE (GspellAppContent, gspell_app_content,
		      GSPELL, APP_CONTENT,
		      GtkGrid)

struct _GspellAppContent
{
	GtkGrid parent;

	GtkTextView *view;
};

G_DEFINE_TYPE (GspellAppContent, gspell_app_content, GTK_TYPE_GRID)

static GspellChecker *
get_spell_checker (GspellAppContent *content)
{
	GtkTextBuffer *gtk_buffer;
	GspellTextBuffer *gspell_buffer;

	gtk_buffer = gtk_text_view_get_buffer (content->view);
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (gtk_buffer);

	return gspell_text_buffer_get_spell_checker (gspell_buffer);
}

static void
gspell_app_content_class_init (GspellAppContentClass *klass)
{
}

static GtkWidget *
get_sidebar (GspellAppContent *content)
{
	GtkWidget *sidebar;
	GtkWidget *language_button;
	GspellChecker *checker;
	const GspellLanguage *language;

	sidebar = gtk_grid_new ();

	g_object_set (sidebar,
		      "margin", 6,
		      NULL);

	gtk_orientable_set_orientation (GTK_ORIENTABLE (sidebar),
					GTK_ORIENTATION_VERTICAL);

	gtk_grid_set_row_spacing (GTK_GRID (sidebar), 6);

	/* Button to launch a language dialog */
	checker = get_spell_checker (content);
	language = gspell_checker_get_language (checker);
	language_button = gspell_language_chooser_button_new (language);
	gtk_container_add (GTK_CONTAINER (sidebar),
			   language_button);

	g_object_bind_property (language_button, "language",
				checker, "language",
				G_BINDING_BIDIRECTIONAL);

	return sidebar;
}

static void
init_view (GspellAppContent *content)
{
	GspellTextView *gspell_view;
	GtkStyleContext *style_context;
	GtkCssProvider *css_provider;
	GError *error = NULL;

	g_assert (content->view == NULL);

	content->view = GTK_TEXT_VIEW (gtk_text_view_new ());

	gspell_view = gspell_text_view_get_from_gtk_text_view (content->view);
	gspell_text_view_basic_setup (gspell_view);

	gtk_text_view_set_wrap_mode (content->view, GTK_WRAP_WORD);

	g_object_set (content->view,
		      "top-margin", 6,
		      "right-margin", 6,
		      "bottom-margin", 6,
		      "left-margin", 6,
		      NULL);

	style_context = gtk_widget_get_style_context (GTK_WIDGET (content->view));

	css_provider = gtk_css_provider_new ();
	gtk_css_provider_load_from_data (css_provider,
					 "textview { font-size: 120%; }\n",
					 -1,
					 &error);

	if (error == NULL)
	{
		gtk_style_context_add_provider (style_context,
						GTK_STYLE_PROVIDER (css_provider),
						GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	}
	else
	{
		g_warning ("CSS error: %s", error->message);
		g_clear_error (&error);
	}

	g_object_unref (css_provider);
}

static void
gspell_app_content_init (GspellAppContent *content)
{
	GtkWidget *scrolled_window;

	init_view (content);

	gtk_orientable_set_orientation (GTK_ORIENTABLE (content),
					GTK_ORIENTATION_HORIZONTAL);

	gtk_container_add (GTK_CONTAINER (content), get_sidebar (content));

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);

	/* Overlay scrolling is annoying when trying to place the cursor at the
	 * last character of a line.
	 */
	gtk_scrolled_window_set_overlay_scrolling (GTK_SCROLLED_WINDOW (scrolled_window), FALSE);

	g_object_set (scrolled_window,
		      "expand", TRUE,
		      NULL);

	gtk_container_add (GTK_CONTAINER (scrolled_window), GTK_WIDGET (content->view));
	gtk_container_add (GTK_CONTAINER (content), scrolled_window);

	gtk_widget_show_all (GTK_WIDGET (content));
}

static GspellAppContent *
gspell_app_content_new (const GspellLanguage *lang)
{
	GspellAppContent *content;
	GspellChecker *checker;

	content = g_object_new (GSPELL_TYPE_APP_CONTENT, NULL);

	if (lang != NULL)
	{
		checker = get_spell_checker (content);
		gspell_checker_set_language (checker, lang);
	}

	return content;
}

static gint
app_command_line_cb (GApplication            *app,
		     GApplicationCommandLine *command_line,
		     gpointer                 user_data)
{
	gint argc;
	gchar **argv;
	const GspellLanguage *lang = NULL;
	GtkWidget *window;
	GspellAppContent *content;

	argv = g_application_command_line_get_arguments (command_line, &argc);

	if (argc > 1)
	{
		const gchar *lang_code;

		/* Last parameter */
		lang_code = argv[argc-1];

		if (g_ascii_strcasecmp (lang_code, "fr") == 0)
		{
			/* Just for me, because I'm lazy and I prefer to type
			 * just fr. -- swilmet
			 */
			lang_code = "fr_BE";
		}

		lang = gspell_language_lookup (lang_code);

		if (lang == NULL)
		{
			g_printerr ("No language found for language code “%s”.\n",
				    lang_code);
		}
	}

	window = gtk_application_window_new (GTK_APPLICATION (app));
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

	content = gspell_app_content_new (lang);
	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (content));

	gtk_widget_show (window);

	g_strfreev (argv);
	return 0;
}

static void
setup_i18n (void)
{
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, GSPELL_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
}

static void
print_available_language_codes (void)
{
	const GList *available_languages;
	const GList *l;

	g_print ("Available language codes: ");

	available_languages = gspell_language_get_available ();

	if (available_languages == NULL)
	{
		g_print ("none\n");
		return;
	}

	for (l = available_languages; l != NULL; l = l->next)
	{
		const GspellLanguage *language = l->data;
		g_print ("%s", gspell_language_get_code (language));

		if (l->next != NULL)
		{
			g_print (", ");
		}
	}

	g_print ("\n");
}

int
main (int    argc,
      char **argv)
{
	GtkApplication *app;
	int ret;

	setup_i18n ();
	g_set_prgname ("gspell-app");

	print_available_language_codes ();

	app = gtk_application_new ("org.gnome.gspell-app",
				   G_APPLICATION_HANDLES_COMMAND_LINE);

	g_signal_connect (app,
			  "command-line",
			  G_CALLBACK (app_command_line_cb),
			  NULL);

	ret = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);
	return ret;
}
