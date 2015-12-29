/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2015 - Sébastien Wilmet
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

#include "gspell-text-buffer.h"
#include "gspell-buffer-notifier.h"

/**
 * SECTION:text-buffer
 * @Title: GtkTextBuffer support
 * @See_also: #GspellChecker
 *
 * Spell checking support for #GtkTextBuffer.
 */

#define SPELL_CHECKER_KEY "gspell-text-buffer-spell-checker-key"

/**
 * gspell_text_buffer_set_spell_checker:
 * @buffer: a #GtkTextBuffer.
 * @checker: (nullable): a #GspellChecker, or %NULL to unset the spell checker.
 *
 * Associates a spell checker to a #GtkTextBuffer.
 */
void
gspell_text_buffer_set_spell_checker (GtkTextBuffer *buffer,
				      GspellChecker *checker)
{
	GspellBufferNotifier *notifier;

	g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
	g_return_if_fail (checker == NULL || GSPELL_IS_CHECKER (checker));

	if (checker != NULL)
	{
		g_object_set_data_full (G_OBJECT (buffer),
					SPELL_CHECKER_KEY,
					g_object_ref (checker),
					g_object_unref);
	}
	else
	{
		g_object_set_data (G_OBJECT (buffer),
				   SPELL_CHECKER_KEY,
				   NULL);
	}

	notifier = _gspell_buffer_notifier_get_instance ();
	_gspell_buffer_notifier_text_buffer_checker_changed (notifier, buffer, checker);
}

/**
 * gspell_text_buffer_get_spell_checker:
 * @buffer: a #GtkTextBuffer.
 *
 * Returns: (nullable) (transfer none): the associated #GspellChecker if one has
 * been set, or %NULL.
 */
GspellChecker *
gspell_text_buffer_get_spell_checker (GtkTextBuffer *buffer)
{
	gpointer data;

	g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);

	data = g_object_get_data (G_OBJECT (buffer), SPELL_CHECKER_KEY);

	if (data == NULL)
	{
		return NULL;
	}

	g_return_val_if_fail (GSPELL_IS_CHECKER (data), NULL);
	return data;
}

/* ex:set ts=8 noet: */
