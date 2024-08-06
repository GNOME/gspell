/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet <swilmet@gnome.org>
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
	GtkEntry *gtk_entry;
	GspellEntry *gspell_entry;

	gtk_entry = GTK_ENTRY (gtk_entry_new ());
	gtk_widget_set_hexpand (GTK_WIDGET (gtk_entry), TRUE);
	gtk_widget_set_valign (GTK_WIDGET (gtk_entry), GTK_ALIGN_START);

	gspell_entry = gspell_entry_get_from_gtk_entry (gtk_entry);
	gspell_entry_basic_setup (gspell_entry);

	return gtk_entry;
}

static void
bold_toggled_cb (GtkToggleButton *button,
		 TestSpell       *spell)
{
	/* Do not care about other users of the GtkEntry:attributes property. An
	 * application or another library might do something similar, but
	 * GspellEntry should still work.
	 */

	if (gtk_toggle_button_get_active (button))
	{
		PangoAttribute *attr_bold;
		PangoAttrList *attr_list;

		attr_bold = pango_attr_weight_new (PANGO_WEIGHT_BOLD);

		attr_list = pango_attr_list_new ();
		pango_attr_list_insert (attr_list, attr_bold);
		gtk_entry_set_attributes (spell->entry, attr_list);
		pango_attr_list_unref (attr_list);
	}
	else
	{
		gtk_entry_set_attributes (spell->entry, NULL);
	}
}

static GtkWidget *
create_sidebar (TestSpell *spell)
{
	GspellEntry *gspell_entry;
	GtkWidget *vgrid;
	GtkWidget *enable_toggle_button;
	GtkWidget *bold_toggle_button;
	GtkWidget *password_toggle_button;

	vgrid = gtk_grid_new ();
	gtk_grid_set_row_spacing (GTK_GRID (vgrid), 6);
	gtk_orientable_set_orientation (GTK_ORIENTABLE (vgrid),
					GTK_ORIENTATION_VERTICAL);

	enable_toggle_button = gtk_toggle_button_new_with_label ("Enable spell-checking");
	gtk_container_add (GTK_CONTAINER (vgrid), enable_toggle_button);

	gspell_entry = gspell_entry_get_from_gtk_entry (spell->entry);
	g_object_bind_property (gspell_entry, "inline-spell-checking",
				enable_toggle_button, "active",
				G_BINDING_BIDIRECTIONAL | G_BINDING_SYNC_CREATE);

	bold_toggle_button = gtk_toggle_button_new_with_label ("Text in bold");
	gtk_container_add (GTK_CONTAINER (vgrid), bold_toggle_button);

	g_signal_connect (bold_toggle_button,
			  "toggled",
			  G_CALLBACK (bold_toggled_cb),
			  spell);

	password_toggle_button = gtk_toggle_button_new_with_label ("Password mode");
	gtk_container_add (GTK_CONTAINER (vgrid), password_toggle_button);

	g_object_bind_property (spell->entry, "visibility",
				password_toggle_button, "active",
				G_BINDING_BIDIRECTIONAL |
				G_BINDING_SYNC_CREATE |
				G_BINDING_INVERT_BOOLEAN);

	gtk_widget_show_all (vgrid);

	return vgrid;
}

static void
test_spell_init (TestSpell *spell)
{
	g_object_set (spell,
		      "margin", 6,
		      NULL);

	gtk_grid_set_column_spacing (GTK_GRID (spell), 6);
	gtk_orientable_set_orientation (GTK_ORIENTABLE (spell),
					GTK_ORIENTATION_HORIZONTAL);

	spell->entry = create_entry ();

	gtk_container_add (GTK_CONTAINER (spell),
			   create_sidebar (spell));

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
