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

#include "gspell-entry-utils.h"
#include <string.h>

GspellEntryWord *
_gspell_entry_word_new (void)
{
	return g_new0 (GspellEntryWord, 1);
}

void
_gspell_entry_word_free (gpointer data)
{
	GspellEntryWord *word = data;

	if (word != NULL)
	{
		g_free (word->word_str);
		g_free (word);
	}
}

/* Returns: (transfer full) (element-type GspellEntryWord): free with
 * g_slist_free_full (words, _gspell_entry_word_free);
 */
GSList *
_gspell_entry_utils_get_words (GtkEntry *entry)
{
	PangoLayout *layout;
	const gchar *text;
	const gchar *cur_text_pos;
	const gchar *word_start;
	gint word_start_char_pos;
	const PangoLogAttr *attrs;
	gint n_attrs = 0;
	gint attr_num;
	GSList *list = NULL;

	g_return_val_if_fail (GTK_IS_ENTRY (entry), NULL);

	layout = gtk_entry_get_layout (entry);
	text = gtk_entry_get_text (entry);
	attrs = pango_layout_get_log_attrs_readonly (layout, &n_attrs);

	attr_num = 0;
	cur_text_pos = text;
	word_start = NULL;
	word_start_char_pos = 0;

	while (attr_num < n_attrs)
	{
		if (word_start != NULL &&
		    attrs[attr_num].is_word_end)
		{
			const gchar *word_end;
			GspellEntryWord *word;

			if (cur_text_pos != NULL)
			{
				word_end = cur_text_pos;
			}
			else
			{
				word_end = word_start + strlen (word_start);
			}

			word = _gspell_entry_word_new ();
			word->byte_start = word_start - text;
			word->byte_end = word_end - text;
			word->char_start = word_start_char_pos;
			word->char_end = attr_num;
			word->word_str = g_strndup (word_start, word_end - word_start);

			list = g_slist_prepend (list, word);

			/* Find next word start. */
			word_start = NULL;
		}

		if (word_start == NULL &&
		    attrs[attr_num].is_word_start)
		{
			word_start = cur_text_pos;
			word_start_char_pos = attr_num;
		}

		if (cur_text_pos == NULL &&
		    attr_num != n_attrs - 1)
		{
			g_warning ("%s(): problem in loop iteration, attr_num=%d but should be %d.",
				   G_STRFUNC,
				   attr_num,
				   n_attrs - 1);
			break;
		}

		attr_num++;
		cur_text_pos = g_utf8_find_next_char (cur_text_pos, NULL);
	}

	return g_slist_reverse (list);
}

/* ex:set ts=8 noet: */
