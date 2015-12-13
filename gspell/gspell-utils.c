/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2010 - Jesse van den Kieboom
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

#include "gspell-utils.h"
#include <string.h>

gboolean
_gspell_utils_is_digit (const gchar *text,
			gssize       text_length)
{
	const gchar *p;
	const gchar *end;

	g_return_val_if_fail (text != NULL, FALSE);
	g_return_val_if_fail (text_length >= -1, FALSE);

	if (text_length == -1)
	{
		text_length = strlen (text);
	}

	p = text;
	end = text + text_length;

	while (p != NULL && *p != '\0')
	{
		gunichar c = g_utf8_get_char (p);

		if (!g_unichar_isdigit (c) && c != '.' && c != ',')
		{
			return FALSE;
		}

		p = g_utf8_find_next_char (p, end);
	}

	return TRUE;
}

GtkTextTag *
_gspell_utils_get_no_spell_check_tag (GtkTextBuffer *buffer)
{
	GtkTextTagTable *tag_table;

	g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);

	tag_table = gtk_text_buffer_get_tag_table (buffer);

	return gtk_text_tag_table_lookup (tag_table, "gtksourceview:context-classes:no-spell-check");
}

gboolean
_gspell_utils_skip_no_spell_check (GtkTextTag        *no_spell_check_tag,
				   GtkTextIter       *start,
				   const GtkTextIter *end)
{
	g_return_val_if_fail (start != NULL, FALSE);
	g_return_val_if_fail (end != NULL, FALSE);

	if (no_spell_check_tag == NULL)
	{
		return TRUE;
	}

	g_return_val_if_fail (GTK_IS_TEXT_TAG (no_spell_check_tag), FALSE);

	while (gtk_text_iter_has_tag (start, no_spell_check_tag))
	{
		GtkTextIter last = *start;

		if (!gtk_text_iter_forward_to_tag_toggle (start, no_spell_check_tag))
		{
			return FALSE;
		}

		if (gtk_text_iter_compare (start, &last) <= 0)
		{
			return FALSE;
		}

		gtk_text_iter_forward_word_end (start);
		gtk_text_iter_backward_word_start (start);

		if (gtk_text_iter_compare (start, &last) <= 0)
		{
			return FALSE;
		}

		if (gtk_text_iter_compare (start, end) >= 0)
		{
			return FALSE;
		}
	}

	return TRUE;
}

/* ex:set ts=8 noet: */
