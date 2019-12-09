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

#include "gspell-entry-utils.h"
#include <string.h>
#include "gspell-utils.h"

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

/* Without the preedit string.
 * Free @log_attrs with g_free().
 */
static void
get_pango_log_attrs (GtkEntry      *entry,
		     PangoLogAttr **log_attrs,
		     gint          *n_attrs)
{
	GtkEntryBuffer *buffer;
	const gchar *text;

	g_assert (log_attrs != NULL);
	g_assert (n_attrs != NULL);

	buffer = gtk_entry_get_buffer (entry);
	text = gtk_entry_buffer_get_text (buffer);

	*n_attrs = gtk_entry_buffer_get_length (buffer) + 1;
	*log_attrs = g_new0 (PangoLogAttr, *n_attrs);

	pango_get_log_attrs (text,
			     gtk_entry_buffer_get_bytes (buffer),
			     -1,
			     NULL,
			     *log_attrs,
			     *n_attrs);

	_gspell_utils_improve_word_boundaries (text, *log_attrs, *n_attrs);
}

/* Returns: (transfer full) (element-type GspellEntryWord): the list of words in
 * @entry, without the preedit string. Free with
 * g_slist_free_full (words, _gspell_entry_word_free);
 *
 * The preedit string is not included, because the current word being typed
 * should not be marked as misspelled, so it doesn't change whether the preedit
 * string is included or not, and the code is simpler without.
 */
GSList *
_gspell_entry_utils_get_words (GtkEntry *entry)
{
	const gchar *text;
	const gchar *cur_text_pos;
	const gchar *word_start;
	gint word_start_char_pos;
	PangoLogAttr *attrs;
	gint n_attrs;
	gint attr_num;
	GSList *list = NULL;

	g_return_val_if_fail (GTK_IS_ENTRY (entry), NULL);

	text = gtk_entry_get_text (entry);

	if (text == NULL || text[0] == '\0')
	{
		return NULL;
	}

	get_pango_log_attrs (entry, &attrs, &n_attrs);

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

		if (attr_num == n_attrs - 1 ||
		    cur_text_pos == NULL ||
		    cur_text_pos[0] == '\0')
		{
			break;
		}

		attr_num++;
		cur_text_pos = g_utf8_find_next_char (cur_text_pos, NULL);
	}

	/* Sanity checks */

	if (attr_num != n_attrs - 1)
	{
		g_warning ("%s(): problem in loop iteration, attr_num=%d but should be %d. "
			   "End of string reached too early.",
			   G_STRFUNC,
			   attr_num,
			   n_attrs - 1);
	}

	if (cur_text_pos != NULL && cur_text_pos[0] != '\0')
	{
		g_warning ("%s(): end of string not reached.", G_STRFUNC);
	}

	g_free (attrs);
	return g_slist_reverse (list);
}

static gint
get_layout_index (GtkEntry *entry,
		  gint      x)
{
	PangoLayout *layout;
	PangoLayoutLine *line;
	gint layout_index; /* in bytes */
	gint trailing_chars;
	const gchar *layout_text;
	const gchar *pos_in_layout_text;
	gint layout_text_byte_length;
	gint max_trailing_chars;

	layout = gtk_entry_get_layout (entry);
	line = pango_layout_get_line_readonly (layout, 0);

	pango_layout_line_x_to_index (line,
				      x * PANGO_SCALE,
				      &layout_index,
				      &trailing_chars);

	layout_text = pango_layout_get_text (layout);

	/* Performance should not be a problem here, it's better too much
	 * security than too few.
	 */
	layout_text_byte_length = strlen (layout_text);
	if (layout_index >= layout_text_byte_length)
	{
		return layout_text_byte_length;
	}

	if (trailing_chars == 0)
	{
		return layout_index;
	}

	pos_in_layout_text = layout_text + layout_index;
	max_trailing_chars = g_utf8_strlen (pos_in_layout_text, -1);
	trailing_chars = MIN (trailing_chars, max_trailing_chars);

	pos_in_layout_text = g_utf8_offset_to_pointer (pos_in_layout_text, trailing_chars);

	return pos_in_layout_text - layout_text;
}

/* The return value is in characters, not bytes. And a position suitable for the
 * text in the GtkEntryBuffer, i.e. without the preedit string.
 */
gint
_gspell_entry_utils_get_char_position_at_event (GtkEntry       *entry,
						GdkEventButton *event)
{
	gint scroll_offset;
	gint x;
	gint layout_index; /* in bytes */
	gint text_index; /* in bytes */
	const gchar *buffer_text;

	g_object_get (entry,
		      "scroll-offset", &scroll_offset,
		      NULL);

	x = event->x + scroll_offset;

	layout_index = get_layout_index (entry, x);
	text_index = gtk_entry_layout_index_to_text_index (entry, layout_index);

	buffer_text = gtk_entry_get_text (entry);
	return g_utf8_pointer_to_offset (buffer_text, buffer_text + text_index);
}

/* ex:set ts=8 noet: */
