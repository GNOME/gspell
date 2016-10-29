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

#define TEST_TYPE_SPELL (test_spell_get_type ())
G_DECLARE_FINAL_TYPE (TestSpell, test_spell,
		      TEST, SPELL,
		      GtkGrid)

struct _TestSpell
{
	GtkGrid parent;

	GtkEntry *entry;
};

G_DEFINE_TYPE (TestSpell, test_spell, GTK_TYPE_GRID)

static void
test_spell_class_init (TestSpellClass *klass)
{
}

static GtkEntry *
create_entry (void)
{
	GspellChecker *checker;
	GtkEntry *gtk_entry;
	GtkEntryBuffer *gtk_buffer;
	GspellEntryBuffer *gspell_buffer;
	GspellEntry *gspell_entry;

	gtk_entry = GTK_ENTRY (gtk_entry_new ());
	gtk_widget_set_hexpand (GTK_WIDGET (gtk_entry), TRUE);

	checker = gspell_checker_new (NULL);
	gtk_buffer = gtk_entry_get_buffer (gtk_entry);
	gspell_buffer = gspell_entry_buffer_get_from_gtk_entry_buffer (gtk_buffer);
	gspell_entry_buffer_set_spell_checker (gspell_buffer, checker);
	g_object_unref (checker);

	gspell_entry = gspell_entry_get_from_gtk_entry (gtk_entry);
	gspell_entry_set_inline_spell_checking (gspell_entry, TRUE);

	return gtk_entry;
}

static void
test_spell_init (TestSpell *spell)
{
	g_object_set (spell,
		      "margin", 6,
		      NULL);

	spell->entry = create_entry ();
	gtk_container_add (GTK_CONTAINER (spell),
			   GTK_WIDGET (spell->entry));

	gtk_widget_show_all (GTK_WIDGET (spell));
}

static TestSpell *
test_spell_new (void)
{
	return g_object_new (TEST_TYPE_SPELL, NULL);
}

gint
main (gint    argc,
      gchar **argv)
{
	GtkWidget *window;
	TestSpell *spell;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	gtk_window_set_default_size (GTK_WINDOW (window), 400, 200);

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
