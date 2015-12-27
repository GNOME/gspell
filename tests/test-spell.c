/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2015 - Sébastien Wilmet <swilmet@gnome.org>
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

#define TEST_TYPE_SPELL (test_spell_get_type ())
G_DECLARE_FINAL_TYPE (TestSpell, test_spell,
		      TEST, SPELL,
		      GtkGrid)

struct _TestSpell
{
	GtkGrid parent;

	GtkTextView *view;
	GspellChecker *checker;
	GspellInlineCheckerGtv *inline_spell;
};

G_DEFINE_TYPE (TestSpell, test_spell, GTK_TYPE_GRID)

static void
test_spell_dispose (GObject *object)
{
	TestSpell *spell = TEST_SPELL (object);

	g_clear_object (&spell->checker);
	g_clear_object (&spell->inline_spell);

	G_OBJECT_CLASS (test_spell_parent_class)->dispose (object);
}

static void
test_spell_class_init (TestSpellClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = test_spell_dispose;
}

static void
checker_button_clicked_cb (GtkButton *checker_button,
			   TestSpell *spell)
{
	GtkWidget *window;
	GtkWidget *checker_dialog;
	GspellNavigator *navigator;

	window = gtk_widget_get_toplevel (GTK_WIDGET (spell));
	if (!gtk_widget_is_toplevel (window))
	{
		g_return_if_reached ();
	}

	navigator = gspell_navigator_gtv_new (spell->view, spell->checker);

	checker_dialog = gspell_checker_dialog_new (GTK_WINDOW (window), navigator);
	g_object_unref (navigator);

	gtk_dialog_run (GTK_DIALOG (checker_dialog));
	gtk_widget_destroy (checker_dialog);
}

static void
highlight_checkbutton_toggled_cb (GtkToggleButton *checkbutton,
				  TestSpell       *spell)
{
	if (gtk_toggle_button_get_active (checkbutton))
	{
		GtkTextBuffer *buffer;

		g_assert (spell->inline_spell == NULL);

		buffer = gtk_text_view_get_buffer (spell->view);

		/* A real application needs to check if
		 * gspell_checker_get_language() != NULL. If it is NULL, the
		 * inline spell checker should not be created and a warning
		 * should be printed to say that no dictionaries are available.
		 */
		spell->inline_spell = gspell_inline_checker_gtv_new (buffer, spell->checker);

		gspell_inline_checker_gtv_attach_view (spell->inline_spell, spell->view);
	}
	else
	{
		g_clear_object (&spell->inline_spell);
	}
}

static GtkWidget *
get_sidebar (TestSpell *spell)
{
	GtkWidget *sidebar;
	GtkWidget *checker_button;
	GtkWidget *language_button;
	GtkWidget *highlight_checkbutton;
	const GspellLanguage *language;

	sidebar = gtk_grid_new ();

	g_object_set (sidebar,
		      "margin", 6,
		      NULL);

	gtk_orientable_set_orientation (GTK_ORIENTABLE (sidebar),
					GTK_ORIENTATION_VERTICAL);

	gtk_grid_set_row_spacing (GTK_GRID (sidebar), 6);

	/* Button to launch a spell checker dialog */
	checker_button = gtk_button_new_with_mnemonic ("_Check Spelling…");
	gtk_container_add (GTK_CONTAINER (sidebar),
			   checker_button);

	g_signal_connect (checker_button,
			  "clicked",
			  G_CALLBACK (checker_button_clicked_cb),
			  spell);

	/* Button to launch a language dialog */
	language = gspell_checker_get_language (spell->checker);
	language_button = gspell_language_chooser_button_new (language);
	gtk_container_add (GTK_CONTAINER (sidebar),
			   language_button);

	g_object_bind_property (language_button, "language",
				spell->checker, "language",
				G_BINDING_DEFAULT);

	/* Checkbutton to activate the inline spell checker */
	highlight_checkbutton = gtk_check_button_new_with_mnemonic ("_Highlight Misspelled Words");
	gtk_container_add (GTK_CONTAINER (sidebar),
			   highlight_checkbutton);

	g_signal_connect (highlight_checkbutton,
			  "toggled",
			  G_CALLBACK (highlight_checkbutton_toggled_cb),
			  spell);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (highlight_checkbutton), TRUE);

	return sidebar;
}

static void
test_spell_init (TestSpell *spell)
{
	GtkWidget *scrolled_window;

	spell->view = GTK_TEXT_VIEW (gtk_text_view_new ());
	spell->checker = gspell_checker_new (NULL);

	gtk_orientable_set_orientation (GTK_ORIENTABLE (spell),
					GTK_ORIENTATION_HORIZONTAL);

	gtk_container_add (GTK_CONTAINER (spell),
			   get_sidebar (spell));

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);

	g_object_set (scrolled_window,
		      "expand", TRUE,
		      NULL);

	gtk_container_add (GTK_CONTAINER (scrolled_window),
			   GTK_WIDGET (spell->view));
	gtk_container_add (GTK_CONTAINER (spell),
			   scrolled_window);

	gtk_widget_show_all (GTK_WIDGET (spell));
}

static TestSpell *
test_spell_new (void)
{
	return g_object_new (TEST_TYPE_SPELL, NULL);
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

gint
main (gint    argc,
      gchar **argv)
{
	GtkWidget *window;
	TestSpell *spell;

	gtk_init (&argc, &argv);

	print_available_language_codes ();

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_window_set_default_size (GTK_WINDOW (window), 800, 600);

	g_signal_connect (window,
			  "destroy",
			  G_CALLBACK (gtk_main_quit),
			  NULL);

	spell = test_spell_new ();
	gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (spell));

	gtk_widget_show (window);

	gtk_main ();

	return 0;
}

/* ex:set ts=8 noet: */
