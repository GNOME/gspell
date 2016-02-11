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
	g_assert (lang != NULL);

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

static void
test_text_insertion (void)
{
	GtkTextBuffer *buffer;
	GspellInlineCheckerTextBuffer *inline_checker;
	GtkTextIter iter;

	buffer = create_buffer ();
	inline_checker = _gspell_inline_checker_text_buffer_new (buffer);
	_gspell_inline_checker_text_buffer_set_unit_test_mode (inline_checker, TRUE);

	gtk_text_buffer_get_start_iter (buffer, &iter);
	gtk_text_buffer_insert (buffer, &iter, "Hello jlyxdt, grrimbl?\nNo, not really today.", -1);

	check_highlighted_words (buffer,
				 inline_checker,
				 6, 12,
				 14, 21,
				 -1);

	/* Modify a word: correctly spelled -> misspelled. */
	gtk_text_buffer_get_start_iter (buffer, &iter);
	gtk_text_buffer_insert (buffer, &iter, "ZK", -1);

	check_highlighted_words (buffer,
				 inline_checker,
				 0, 7,
				 8, 14,
				 16, 23,
				 -1);

	/* Insert new misspelled word. */
	gtk_text_buffer_get_start_iter (buffer, &iter);
	gtk_text_buffer_insert (buffer, &iter, "Tst ", -1);

	check_highlighted_words (buffer,
				 inline_checker,
				 0, 3,
				 4, 11,
				 12, 18,
				 20, 27,
				 -1);

	/* Modify word: misspelled -> correctly spelled. */
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 1);
	gtk_text_buffer_insert (buffer, &iter, "e", -1);

	check_highlighted_words (buffer,
				 inline_checker,
				 5, 12,
				 13, 19,
				 21, 28,
				 -1);

	/* Have two collapsed correctly spelled words ("Testagain"). */
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 4);
	gtk_text_buffer_insert (buffer, &iter, "again", -1);

	check_highlighted_words (buffer,
				 inline_checker,
				 0, 9,
				 10, 17,
				 18, 24,
				 26, 33,
				 -1);

	/* Test neighbor words: "Testagain" -> "Test again". */
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 4);
	gtk_text_buffer_insert (buffer, &iter, " ", -1);

	check_highlighted_words (buffer,
				 inline_checker,
				 11, 18,
				 19, 25,
				 27, 34,
				 -1);

	g_object_unref (inline_checker);
	g_object_unref (buffer);
}

static void
test_text_deletion (void)
{
	GtkTextBuffer *buffer;
	GspellInlineCheckerTextBuffer *inline_checker;
	GtkTextIter start;
	GtkTextIter end;

	buffer = create_buffer ();
	gtk_text_buffer_set_text (buffer, "Test againnn againnn.", -1);

	inline_checker = _gspell_inline_checker_text_buffer_new (buffer);
	_gspell_inline_checker_text_buffer_set_unit_test_mode (inline_checker, TRUE);

	check_highlighted_words (buffer,
				 inline_checker,
				 5, 12,
				 13, 20,
				 -1);

	/* Modify end of word: misspelled -> still misspelled.
	 * First "againnn" -> "againn"
	 */
	gtk_text_buffer_get_iter_at_offset (buffer, &start, 11);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, 12);
	gtk_text_buffer_delete (buffer, &start, &end);

	check_highlighted_words (buffer,
				 inline_checker,
				 5, 11,
				 12, 19,
				 -1);

	/* Modify end of word: misspelled -> correctly spelled.
	 * "againn" -> "again"
	 */
	gtk_text_buffer_get_iter_at_offset (buffer, &start, 10);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, 11);
	gtk_text_buffer_delete (buffer, &start, &end);

	check_highlighted_words (buffer,
				 inline_checker,
				 11, 18,
				 -1);

	/* Modify middle of word: misspelled -> correctly spelled.
	 * Second "agai[nn]n" -> "again"
	 */
	gtk_text_buffer_get_iter_at_offset (buffer, &start, 15);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, 17);
	gtk_text_buffer_delete (buffer, &start, &end);

	check_highlighted_words (buffer, inline_checker, -1);

	/* Modify word: correctly spelled -> misspelled.
	 * Second "again" -> "agan"
	 */
	gtk_text_buffer_get_iter_at_offset (buffer, &start, 14);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, 15);
	gtk_text_buffer_delete (buffer, &start, &end);

	check_highlighted_words (buffer,
				 inline_checker,
				 11, 15,
				 -1);

	/* Collapse two correctly spelled words to form a misspelled word.
	 * "Test again" -> "Testagain"
	 */
	gtk_text_buffer_get_iter_at_offset (buffer, &start, 4);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, 5);
	gtk_text_buffer_delete (buffer, &start, &end);

	check_highlighted_words (buffer,
				 inline_checker,
				 0, 9,
				 10, 14,
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

	g_test_add_func ("/inline-checker-text-buffer/text-insertion",
			 test_text_insertion);

	g_test_add_func ("/inline-checker-text-buffer/text-deletion",
			 test_text_deletion);

	return g_test_run ();
}
