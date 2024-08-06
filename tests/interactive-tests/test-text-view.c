/* SPDX-FileCopyrightText: 2015 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
};

G_DEFINE_TYPE (TestSpell, test_spell, GTK_TYPE_GRID)

static GspellChecker *
get_spell_checker (TestSpell *spell)
{
	GtkTextBuffer *gtk_buffer;
	GspellTextBuffer *gspell_buffer;

	gtk_buffer = gtk_text_view_get_buffer (spell->view);
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (gtk_buffer);

	return gspell_text_buffer_get_spell_checker (gspell_buffer);
}

static void
test_spell_class_init (TestSpellClass *klass)
{
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

	navigator = gspell_navigator_text_view_new (spell->view);
	checker_dialog = gspell_checker_dialog_new (GTK_WINDOW (window), navigator);

	gtk_widget_show (checker_dialog);
}

static void
change_buffer_button_clicked_cb (GtkButton *change_buffer_button,
				 TestSpell *spell)
{
	GtkTextBuffer *old_gtk_buffer;
	GtkTextBuffer *new_gtk_buffer;
	GspellTextBuffer *old_gspell_buffer;
	GspellTextBuffer *new_gspell_buffer;
	GspellChecker *checker;

	old_gtk_buffer = gtk_text_view_get_buffer (spell->view);
	old_gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (old_gtk_buffer);
	checker = gspell_text_buffer_get_spell_checker (old_gspell_buffer);

	new_gtk_buffer = gtk_text_buffer_new (NULL);
	new_gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (new_gtk_buffer);
	gspell_text_buffer_set_spell_checker (new_gspell_buffer, checker);

	gtk_text_view_set_buffer (spell->view, new_gtk_buffer);
	g_object_unref (new_gtk_buffer);
}

static GtkWidget *
get_sidebar (TestSpell *spell)
{
	GtkWidget *sidebar;
	GtkWidget *checker_button;
	GtkWidget *language_button;
	GtkWidget *highlight_checkbutton;
	GtkWidget *change_buffer_button;
	GspellChecker *checker;
	const GspellLanguage *language;
	GspellTextView *gspell_view;

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
	checker = get_spell_checker (spell);
	language = gspell_checker_get_language (checker);
	language_button = gspell_language_chooser_button_new (language);
	gtk_container_add (GTK_CONTAINER (sidebar),
			   language_button);

	g_object_bind_property (language_button, "language",
				checker, "language",
				G_BINDING_BIDIRECTIONAL);

	/* Checkbutton to activate the inline spell checker */
	highlight_checkbutton = gtk_check_button_new_with_mnemonic ("_Highlight Misspelled Words");
	gtk_container_add (GTK_CONTAINER (sidebar),
			   highlight_checkbutton);

	gspell_view = gspell_text_view_get_from_gtk_text_view (spell->view);
	gspell_text_view_set_enable_language_menu (gspell_view, TRUE);

	g_object_bind_property (highlight_checkbutton, "active",
				gspell_view, "inline-spell-checking",
				G_BINDING_DEFAULT);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (highlight_checkbutton), TRUE);

	/* Button to change the GtkTextBuffer */
	change_buffer_button = gtk_button_new_with_mnemonic ("Change _Buffer!");
	gtk_container_add (GTK_CONTAINER (sidebar),
			   change_buffer_button);

	g_signal_connect (change_buffer_button,
			  "clicked",
			  G_CALLBACK (change_buffer_button_clicked_cb),
			  spell);

	return sidebar;
}

static void
test_spell_init (TestSpell *spell)
{
	GtkWidget *scrolled_window;
	GtkTextBuffer *gtk_buffer;
	GspellTextBuffer *gspell_buffer;
	GspellChecker *checker;

	spell->view = GTK_TEXT_VIEW (gtk_text_view_new ());
	gtk_buffer = gtk_text_view_get_buffer (spell->view);

	checker = gspell_checker_new (NULL);
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (gtk_buffer);
	gspell_text_buffer_set_spell_checker (gspell_buffer, checker);
	g_object_unref (checker);

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
