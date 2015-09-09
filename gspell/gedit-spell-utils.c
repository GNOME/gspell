/*
 * This file is part of gspell.
 *
 * Copyright 2010 - Jesse van den Kieboom
 * Copyright 2015 - Sébastien Wilmet
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "gedit-spell-utils.h"
#include <string.h>
#include <gtksourceview/gtksource.h>

gboolean
gedit_spell_utils_is_digit (const gchar *text)
{
	const gchar *p;
	const gchar *end;

	g_return_val_if_fail (text != NULL, FALSE);

	p = text;
	end = text + strlen (text);

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

gboolean
gedit_spell_utils_skip_no_spell_check (GtkTextIter       *start,
				       const GtkTextIter *end)
{
	GtkSourceBuffer *buffer = GTK_SOURCE_BUFFER (gtk_text_iter_get_buffer (start));

	while (gtk_source_buffer_iter_has_context_class (buffer, start, "no-spell-check"))
	{
		GtkTextIter last = *start;

		if (!gtk_source_buffer_iter_forward_to_context_class_toggle (buffer, start, "no-spell-check"))
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
