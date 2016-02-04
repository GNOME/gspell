/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#include <gspell/gspell.h>
#include "gspell/gspell-inline-checker-text-buffer.h"

static GtkTextBuffer *
create_buffer (void)
{
	GtkTextBuffer *buffer;
	const GspellLanguage *lang;
	GspellChecker *checker;

	buffer = gtk_text_buffer_new (NULL);

	lang = gspell_language_lookup ("en_US");
	checker = gspell_checker_new (lang);
	gspell_text_buffer_set_spell_checker (buffer, checker);
	g_object_unref (checker);

	return buffer;
}

/*
 * check_highlighted_words:
 * @buffer:
 * @inline_checker:
 * @...: list of pairs of character offsets (as #gint's) delimiting the expected
 * highlighted words in @buffer. The list must be terminated by -1.
 *
 * Checks the boundaries of the highlighted words in @buffer.
 */
static void
check_highlighted_words (GtkTextBuffer                 *buffer,
			 GspellInlineCheckerTextBuffer *inline_checker,
			 ...)
{
	GtkTextTag *tag;
	GtkTextIter iter;
	va_list list;
	gint remaining_list_element;

	tag = _gspell_inline_checker_text_buffer_get_highlight_tag (inline_checker);
	g_assert (GTK_IS_TEXT_TAG (tag));

	gtk_text_buffer_get_start_iter (buffer, &iter);
	va_start (list, inline_checker);

	while (!gtk_text_iter_is_end (&iter))
	{
		GtkTextIter start;
		GtkTextIter end;
		gint start_offset;
		gint end_offset;
		gint expected_start_offset;
		gint expected_end_offset;

		start = iter;
		if (!gtk_text_iter_starts_tag (&start, tag))
		{
			if (!gtk_text_iter_forward_to_tag_toggle (&start, tag))
			{
				break;
			}

			g_assert (gtk_text_iter_starts_tag (&start, tag));
		}

		end = start;
		if (!gtk_text_iter_forward_to_tag_toggle (&end, tag))
		{
			g_assert_not_reached ();
		}
		g_assert (gtk_text_iter_ends_tag (&end, tag));

		start_offset = gtk_text_iter_get_offset (&start);
		expected_start_offset = va_arg (list, gint);
		g_assert_cmpint (expected_start_offset, ==, start_offset);

		end_offset = gtk_text_iter_get_offset (&end);
		expected_end_offset = va_arg (list, gint);
		g_assert_cmpint (expected_end_offset, ==, end_offset);

		iter = end;
	}

	remaining_list_element = va_arg (list, gint);
	g_assert_cmpint (remaining_list_element, ==, -1);

	va_end (list);
}

static void
test_whole_buffer (void)
{
	GtkTextBuffer *buffer;
	GspellInlineCheckerTextBuffer *inline_checker;

	buffer = create_buffer ();
	gtk_text_buffer_set_text (buffer, "Hello jlyxdt, grrimbl?\nNo, not really today.", -1);

	inline_checker = _gspell_inline_checker_text_buffer_new (buffer);
	_gspell_inline_checker_text_buffer_set_unit_test_mode (inline_checker, TRUE);

	check_highlighted_words (buffer,
				 inline_checker,
				 6, 12,
				 14, 21,
				 -1);

	g_object_unref (inline_checker);
	g_object_unref (buffer);
}

gint
main (gint    argc,
      gchar **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/inline-checker-text-buffer/whole-buffer",
			 test_whole_buffer);

	return g_test_run ();
}
