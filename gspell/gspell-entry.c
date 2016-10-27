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
set_entry (GspellEntry *gspell_entry,
	   GtkEntry    *gtk_entry)
{
	g_return_if_fail (GTK_IS_ENTRY (gtk_entry));

	g_assert (gspell_entry->entry == NULL);
	gspell_entry->entry = gtk_entry;

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
