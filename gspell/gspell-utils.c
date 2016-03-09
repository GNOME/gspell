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
#include "gspell-text-iter.h"

gboolean
_gspell_utils_is_number (const gchar *text,
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

		_gspell_text_iter_forward_word_end (start);
		_gspell_text_iter_backward_word_start (start);

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

/**
 * _gspell_utils_str_replace:
 * @string: a string
 * @search: the search string
 * @replacement: the replacement string
 *
 * Replaces all occurences of @search by @replacement.
 *
 * Returns: A newly allocated string with the replacements. Free with g_free().
 */
gchar *
_gspell_utils_str_replace (const gchar *string,
                           const gchar *search,
                           const gchar *replacement)
{
	gchar **chunks;
	gchar *ret;

	g_return_val_if_fail (string != NULL, NULL);
	g_return_val_if_fail (search != NULL, NULL);
	g_return_val_if_fail (replacement != NULL, NULL);

	chunks = g_strsplit (string, search, -1);
	if (chunks != NULL && chunks[0] != NULL)
	{
		ret = g_strjoinv (replacement, chunks);
	}
	else
	{
		ret = g_strdup (string);
	}

	g_strfreev (chunks);
	return ret;
}

/* Replaces unicode (non-ascii) apostrophes by the ascii apostrophe.
 * Because with unicode apostrophes, the word is marked as misspelled. It should
 * probably be fixed in hunspell, aspell, etc.
 * Returns: %TRUE if @result has been set, %FALSE if @word must be used
 * (to avoid a malloc).
 */
gboolean
_gspell_utils_str_to_ascii_apostrophe (const gchar  *word,
				       gssize        word_length,
				       gchar       **result)
{
	gchar *word_to_free = NULL;
	const gchar *nul_terminated_word;

	g_return_val_if_fail (word != NULL, FALSE);
	g_return_val_if_fail (word_length >= -1, FALSE);
	g_return_val_if_fail (result != NULL, FALSE);

	if (g_utf8_strchr (word, word_length, _GSPELL_MODIFIER_LETTER_APOSTROPHE) == NULL &&
	    g_utf8_strchr (word, word_length, _GSPELL_RIGHT_SINGLE_QUOTATION_MARK) == NULL)
	{
		return FALSE;
	}

	if (word_length == -1)
	{
		nul_terminated_word = word;
	}
	else
	{
		word_to_free = g_strndup (word, word_length);
		nul_terminated_word = word_to_free;
	}

	*result = _gspell_utils_str_replace (nul_terminated_word, "\xCA\xBC", "'");

	g_free (word_to_free);
	word_to_free = *result;
	*result = _gspell_utils_str_replace (*result, "\xE2\x80\x99", "'");

	g_free (word_to_free);
	return TRUE;
}

/* ex:set ts=8 noet: */
