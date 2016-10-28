/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2016 - Sébastien Wilmet
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

#include "gspell-entry.h"
#include "gspell-entry-utils.h"

/**
 * SECTION:entry
 * @Title: GspellEntry
 * @Short_description: Spell checking support for GtkEntry
 *
 * #GspellEntry extends the #GtkEntry class with inline spell checking.
 * Misspelled words are highlighted with a %PANGO_UNDERLINE_ERROR, usually a red
 * wavy underline.
 *
 * Note that #GspellEntry extends the #GtkEntry class but without subclassing
 * it, because #GtkEntry is already subclassed by #GtkSearchEntry for example.
 */

struct _GspellEntry
{
	GObject parent;

	GtkEntry *entry;

	gulong notify_attributes_handler_id;
	guint notify_attributes_idle_id;

	guint inline_spell_checking : 1;
};

enum
{
	PROP_0,
	PROP_ENTRY,
	PROP_INLINE_SPELL_CHECKING,
};

#define GSPELL_ENTRY_KEY "gspell-entry-key"

G_DEFINE_TYPE (GspellEntry, gspell_entry, G_TYPE_OBJECT)

static void
set_attributes (GspellEntry   *gspell_entry,
		PangoAttrList *attributes)
{
	g_signal_handler_block (gspell_entry->entry,
				gspell_entry->notify_attributes_handler_id);

	gtk_entry_set_attributes (gspell_entry->entry, attributes);

	g_signal_handler_unblock (gspell_entry->entry,
				  gspell_entry->notify_attributes_handler_id);
}

static gboolean
remove_underlines_filter (PangoAttribute *attr,
			  gpointer        user_data)
{
	return (attr->klass->type == PANGO_ATTR_UNDERLINE ||
		attr->klass->type == PANGO_ATTR_UNDERLINE_COLOR);
}

static void
remove_all_underlines (GspellEntry *gspell_entry)
{
	PangoAttrList *attr_list;

	attr_list = gtk_entry_get_attributes (gspell_entry->entry);

	if (attr_list == NULL)
	{
		return;
	}

	pango_attr_list_filter (attr_list,
				remove_underlines_filter,
				NULL);
}

static void
insert_underline (GspellEntry *gspell_entry,
		  guint        byte_start,
		  guint        byte_end)
{
	PangoAttribute *attr_underline;
	PangoAttribute *attr_underline_color;
	PangoAttrList *attr_list;

	attr_underline = pango_attr_underline_new (PANGO_UNDERLINE_ERROR);
	attr_underline->start_index = byte_start;
	attr_underline->end_index = byte_end;

	attr_underline_color = pango_attr_underline_color_new (65535, 0, 0);
	attr_underline_color->start_index = byte_start;
	attr_underline_color->end_index = byte_end;

	attr_list = gtk_entry_get_attributes (gspell_entry->entry);

	if (attr_list == NULL)
	{
		attr_list = pango_attr_list_new ();
		set_attributes (gspell_entry, attr_list);
		pango_attr_list_unref (attr_list);
	}

	/* Do not use pango_attr_list_change(), because all previous underlines
	 * are anyway removed by remove_all_underlines().
	 */
	pango_attr_list_insert (attr_list, attr_underline);
	pango_attr_list_insert (attr_list, attr_underline_color);
}

static void
recheck_all (GspellEntry *gspell_entry)
{
	PangoAttrList *attr_list;
	GSList *words;
	GSList *l;

	words = _gspell_entry_utils_get_words (gspell_entry->entry);

	for (l = words; l != NULL; l = l->next)
	{
		GspellEntryWord *cur_word = l->data;

		insert_underline (gspell_entry,
				  cur_word->byte_start,
				  cur_word->byte_end);
	}

	g_slist_free_full (words, _gspell_entry_word_free);

	/* If more attributes have been added to an existing PangoAttrList,
	 * GtkEntry doesn't know that the :attributes property has been
	 * modified. Without this code, GtkEntry can become buggy, especially
	 * with multi-byte characters (displaying them as unknown char boxes).
	 */
	attr_list = gtk_entry_get_attributes (gspell_entry->entry);
	set_attributes (gspell_entry, attr_list);
}

/* Connect to the ::changed signal before/after, so that other features (in
 * other libraries or apps) can insert other underline attributes (e.g. for
 * grammar checking) in another phase of the ::changed signal emission.
 *
 * But the GtkEntry API is not as nice as the GtkTextView API for inserting
 * tags, setting priorities on them, etc. So, do the best that we can with the
 * current GtkEntry API. If you find a better solution...
 */
static void
changed_before_cb (GtkEditable *editable,
		   GspellEntry *gspell_entry)
{
	remove_all_underlines (gspell_entry);
}

static void
changed_after_cb (GtkEditable *editable,
		  GspellEntry *gspell_entry)
{
	recheck_all (gspell_entry);
}

static gboolean
notify_attributes_idle_cb (gpointer user_data)
{
	GspellEntry *gspell_entry = GSPELL_ENTRY (user_data);

	/* Re-apply our attributes. Do it in an idle function, to not be inside
	 * a notify::attributes signal emission. If we call recheck_all() during
	 * the signal emission, there is an infinite loop.
	 */
	recheck_all (gspell_entry);

	gspell_entry->notify_attributes_idle_id = 0;
	return G_SOURCE_REMOVE;
}

static void
notify_attributes_cb (GtkEntry    *gtk_entry,
		      GParamSpec  *pspec,
		      GspellEntry *gspell_entry)
{
	if (gspell_entry->notify_attributes_idle_id == 0)
	{
		gspell_entry->notify_attributes_idle_id =
			g_idle_add_full (G_PRIORITY_HIGH_IDLE,
					 notify_attributes_idle_cb,
					 gspell_entry,
					 NULL);
	}
}

static void
set_entry (GspellEntry *gspell_entry,
	   GtkEntry    *gtk_entry)
{
	g_return_if_fail (GTK_IS_ENTRY (gtk_entry));

	g_assert (gspell_entry->entry == NULL);
	gspell_entry->entry = gtk_entry;

	g_signal_connect (gtk_entry,
			  "changed",
			  G_CALLBACK (changed_before_cb),
			  gspell_entry);

	g_signal_connect_after (gtk_entry,
				"changed",
				G_CALLBACK (changed_after_cb),
				gspell_entry);

	g_assert (gspell_entry->notify_attributes_handler_id == 0);
	gspell_entry->notify_attributes_handler_id =
		g_signal_connect (gtk_entry,
				  "notify::attributes",
				  G_CALLBACK (notify_attributes_cb),
				  gspell_entry);

	g_object_notify (G_OBJECT (gspell_entry), "entry");
}

static void
gspell_entry_get_property (GObject    *object,
			   guint       prop_id,
			   GValue     *value,
			   GParamSpec *pspec)
{
	GspellEntry *gspell_entry = GSPELL_ENTRY (object);

	switch (prop_id)
	{
		case PROP_ENTRY:
			g_value_set_object (value, gspell_entry_get_entry (gspell_entry));
			break;

		case PROP_INLINE_SPELL_CHECKING:
			g_value_set_boolean (value, gspell_entry_get_inline_spell_checking (gspell_entry));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_entry_set_property (GObject      *object,
			   guint         prop_id,
			   const GValue *value,
			   GParamSpec   *pspec)
{
	GspellEntry *gspell_entry = GSPELL_ENTRY (object);

	switch (prop_id)
	{
		case PROP_ENTRY:
			set_entry (gspell_entry, g_value_get_object (value));
			break;

		case PROP_INLINE_SPELL_CHECKING:
			gspell_entry_set_inline_spell_checking (gspell_entry, g_value_get_boolean (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_entry_dispose (GObject *object)
{
	GspellEntry *gspell_entry = GSPELL_ENTRY (object);

	gspell_entry->entry = NULL;

	if (gspell_entry->notify_attributes_idle_id != 0)
	{
		g_source_remove (gspell_entry->notify_attributes_idle_id);
		gspell_entry->notify_attributes_idle_id = 0;
	}

	G_OBJECT_CLASS (gspell_entry_parent_class)->dispose (object);
}

static void
gspell_entry_class_init (GspellEntryClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gspell_entry_get_property;
	object_class->set_property = gspell_entry_set_property;
	object_class->dispose = gspell_entry_dispose;

	/**
	 * GspellEntry:entry:
	 *
	 * The #GtkEntry.
	 *
	 * Since: 1.4
	 */
	g_object_class_install_property (object_class,
					 PROP_ENTRY,
					 g_param_spec_object ("entry",
							      "Entry",
							      "",
							      GTK_TYPE_ENTRY,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * GspellEntry:inline-spell-checking:
	 *
	 * Whether the inline spell checking is enabled.
	 *
	 * Since: 1.4
	 */
	g_object_class_install_property (object_class,
					 PROP_INLINE_SPELL_CHECKING,
					 g_param_spec_boolean ("inline-spell-checking",
							       "Inline Spell Checking",
							       "",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));
}

static void
gspell_entry_init (GspellEntry *gspell_entry)
{
}

/**
 * gspell_entry_get_from_gtk_entry:
 * @gtk_entry: a #GtkEntry.
 *
 * Returns the #GspellEntry of @gtk_entry. The returned object is guaranteed
 * to be the same for the lifetime of @gtk_entry.
 *
 * Returns: (transfer none): the #GspellEntry of @gtk_entry.
 * Since: 1.4
 */
GspellEntry *
gspell_entry_get_from_gtk_entry (GtkEntry *gtk_entry)
{
	GspellEntry *gspell_entry;

	g_return_val_if_fail (GTK_IS_ENTRY (gtk_entry), NULL);

	gspell_entry = g_object_get_data (G_OBJECT (gtk_entry), GSPELL_ENTRY_KEY);

	if (gspell_entry == NULL)
	{
		gspell_entry = g_object_new (GSPELL_TYPE_ENTRY,
					     "entry", gtk_entry,
					     NULL);

		g_object_set_data_full (G_OBJECT (gtk_entry),
					GSPELL_ENTRY_KEY,
					gspell_entry,
					g_object_unref);
	}

	g_return_val_if_fail (GSPELL_IS_ENTRY (gspell_entry), NULL);
	return gspell_entry;
}

/**
 * gspell_entry_get_entry:
 * @gspell_entry: a #GspellEntry.
 *
 * Returns: (transfer none): the #GtkEntry of @gspell_entry.
 * Since: 1.4
 */
GtkEntry *
gspell_entry_get_entry (GspellEntry *gspell_entry)
{
	g_return_val_if_fail (GSPELL_IS_ENTRY (gspell_entry), NULL);

	return gspell_entry->entry;
}

/**
 * gspell_entry_get_inline_spell_checking:
 * @gspell_entry: a #GspellEntry.
 *
 * Returns: whether the inline spell checking is enabled.
 * Since: 1.4
 */
gboolean
gspell_entry_get_inline_spell_checking (GspellEntry *gspell_entry)
{
	g_return_val_if_fail (GSPELL_IS_ENTRY (gspell_entry), FALSE);

	return gspell_entry->inline_spell_checking;
}

/**
 * gspell_entry_set_inline_spell_checking:
 * @gspell_entry: a #GspellEntry.
 * @enable: the new state.
 *
 * Enables or disables the inline spell checking.
 *
 * Since: 1.4
 */
void
gspell_entry_set_inline_spell_checking (GspellEntry *gspell_entry,
					gboolean     enable)
{
	g_return_if_fail (GSPELL_IS_ENTRY (gspell_entry));

	enable = enable != FALSE;

	if (gspell_entry->inline_spell_checking != enable)
	{
		gspell_entry->inline_spell_checking = enable;
		g_object_notify (G_OBJECT (gspell_entry), "inline-spell-checking");
	}
}

/* ex:set ts=8 noet: */
