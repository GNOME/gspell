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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gspell-entry-buffer.h"

/**
 * SECTION:entry-buffer
 * @Title: GspellEntryBuffer
 * @Short_description: Spell checking support for GtkEntryBuffer
 *
 * #GspellEntryBuffer extends the #GtkEntryBuffer class with spell checking
 * support.
 */

struct _GspellEntryBuffer
{
	GObject parent;

	GtkEntryBuffer *buffer;
	GspellChecker *spell_checker;
};

enum
{
	PROP_0,
	PROP_BUFFER,
	PROP_SPELL_CHECKER,
};

#define GSPELL_ENTRY_BUFFER_KEY "gspell-entry-buffer-key"

G_DEFINE_TYPE (GspellEntryBuffer, gspell_entry_buffer, G_TYPE_OBJECT)

static void
gspell_entry_buffer_get_property (GObject    *object,
				  guint       prop_id,
				  GValue     *value,
				  GParamSpec *pspec)
{
	GspellEntryBuffer *gspell_buffer = GSPELL_ENTRY_BUFFER (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_value_set_object (value, gspell_entry_buffer_get_buffer (gspell_buffer));
			break;

		case PROP_SPELL_CHECKER:
			g_value_set_object (value, gspell_entry_buffer_get_spell_checker (gspell_buffer));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_entry_buffer_set_property (GObject      *object,
				  guint         prop_id,
				  const GValue *value,
				  GParamSpec   *pspec)
{
	GspellEntryBuffer *gspell_buffer = GSPELL_ENTRY_BUFFER (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_assert (gspell_buffer->buffer == NULL);
			gspell_buffer->buffer = g_value_get_object (value);
			break;

		case PROP_SPELL_CHECKER:
			gspell_entry_buffer_set_spell_checker (gspell_buffer, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_entry_buffer_dispose (GObject *object)
{
	GspellEntryBuffer *gspell_buffer = GSPELL_ENTRY_BUFFER (object);

	gspell_buffer->buffer = NULL;
	g_clear_object (&gspell_buffer->spell_checker);

	G_OBJECT_CLASS (gspell_entry_buffer_parent_class)->dispose (object);
}

static void
gspell_entry_buffer_class_init (GspellEntryBufferClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gspell_entry_buffer_get_property;
	object_class->set_property = gspell_entry_buffer_set_property;
	object_class->dispose = gspell_entry_buffer_dispose;

	/**
	 * GspellEntryBuffer:buffer:
	 *
	 * The #GtkEntryBuffer.
	 *
	 * Since: 1.4
	 */
	g_object_class_install_property (object_class,
					 PROP_BUFFER,
					 g_param_spec_object ("buffer",
							      "Buffer",
							      "",
							      GTK_TYPE_ENTRY_BUFFER,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * GspellEntryBuffer:spell-checker:
	 *
	 * The #GspellChecker.
	 *
	 * Since: 1.4
	 */
	g_object_class_install_property (object_class,
					 PROP_SPELL_CHECKER,
					 g_param_spec_object ("spell-checker",
							      "Spell Checker",
							      "",
							      GSPELL_TYPE_CHECKER,
							      G_PARAM_READWRITE |
							      G_PARAM_STATIC_STRINGS));
}

static void
gspell_entry_buffer_init (GspellEntryBuffer *gspell_buffer)
{
}

/**
 * gspell_entry_buffer_get_from_gtk_entry_buffer:
 * @gtk_buffer: a #GtkEntryBuffer.
 *
 * Returns the #GspellEntryBuffer of @gtk_buffer. The returned object is
 * guaranteed to be the same for the lifetime of @gtk_buffer.
 *
 * Returns: (transfer none): the #GspellEntryBuffer of @gtk_buffer.
 * Since: 1.4
 */
GspellEntryBuffer *
gspell_entry_buffer_get_from_gtk_entry_buffer (GtkEntryBuffer *gtk_buffer)
{
	GspellEntryBuffer *gspell_buffer;

	g_return_val_if_fail (GTK_IS_ENTRY_BUFFER (gtk_buffer), NULL);

	gspell_buffer = g_object_get_data (G_OBJECT (gtk_buffer), GSPELL_ENTRY_BUFFER_KEY);

	if (gspell_buffer == NULL)
	{
		gspell_buffer = g_object_new (GSPELL_TYPE_ENTRY_BUFFER,
					      "buffer", gtk_buffer,
					      NULL);

		g_object_set_data_full (G_OBJECT (gtk_buffer),
					GSPELL_ENTRY_BUFFER_KEY,
					gspell_buffer,
					g_object_unref);
	}

	g_return_val_if_fail (GSPELL_IS_ENTRY_BUFFER (gspell_buffer), NULL);
	return gspell_buffer;
}

/**
 * gspell_entry_buffer_get_buffer:
 * @gspell_buffer: a #GspellEntryBuffer.
 *
 * Returns: (transfer none): the #GtkEntryBuffer of @gspell_buffer.
 * Since: 1.4
 */
GtkEntryBuffer *
gspell_entry_buffer_get_buffer (GspellEntryBuffer *gspell_buffer)
{
	g_return_val_if_fail (GSPELL_IS_ENTRY_BUFFER (gspell_buffer), NULL);

	return gspell_buffer->buffer;
}

/**
 * gspell_entry_buffer_get_spell_checker:
 * @gspell_buffer: a #GspellEntryBuffer.
 *
 * Returns: (nullable) (transfer none): the #GspellChecker if one has been set,
 *   or %NULL.
 * Since: 1.4
 */
GspellChecker *
gspell_entry_buffer_get_spell_checker (GspellEntryBuffer *gspell_buffer)
{
	g_return_val_if_fail (GSPELL_IS_ENTRY_BUFFER (gspell_buffer), NULL);

	return gspell_buffer->spell_checker;
}

/**
 * gspell_entry_buffer_set_spell_checker:
 * @gspell_buffer: a #GspellEntryBuffer.
 * @spell_checker: (nullable): a #GspellChecker, or %NULL to unset the spell
 *   checker.
 *
 * Sets a #GspellChecker to a #GspellEntryBuffer. The @gspell_buffer will own a
 * reference to @spell_checker, so you can release your reference to
 * @spell_checker if you no longer need it.
 *
 * Since: 1.4
 */
void
gspell_entry_buffer_set_spell_checker (GspellEntryBuffer *gspell_buffer,
				       GspellChecker     *spell_checker)
{
	g_return_if_fail (GSPELL_IS_ENTRY_BUFFER (gspell_buffer));
	g_return_if_fail (spell_checker == NULL || GSPELL_IS_CHECKER (spell_checker));

	if (g_set_object (&gspell_buffer->spell_checker, spell_checker))
	{
		g_object_notify (G_OBJECT (gspell_buffer), "spell-checker");
	}
}

/* ex:set ts=8 noet: */
