/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <gspell/gspell.h>
#include "gspell/gspell-inline-checker-text-buffer.h"

static GtkTextBuffer *
create_buffer (void)
{
	GtkTextBuffer *gtk_buffer;
	GspellTextBuffer *gspell_buffer;
	const GspellLanguage *lang;
	GspellChecker *checker;

	gtk_buffer = gtk_text_buffer_new (NULL);
	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (gtk_buffer);

	lang = gspell_language_lookup ("en_US");
	g_assert_true (lang != NULL);

	checker = gspell_checker_new (lang);
	gspell_text_buffer_set_spell_checker (gspell_buffer, checker);
	g_object_unref (checker);

	return gtk_buffer;
}

static void
insert_text_one_char_at_a_time (GtkTextBuffer *buffer,
				GtkTextIter   *iter,
				const gchar   *text)
{
	const gchar *p;

	/* Assumes it's only ASCII characters. */
	for (p = text; p != NULL && *p != '\0'; p++)
	{
		gtk_text_buffer_insert (buffer, iter, p, 1);
	}
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
	g_assert_true (GTK_IS_TEXT_TAG (tag));

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

			g_assert_true (gtk_text_iter_starts_tag (&start, tag));
		}

		end = start;
		if (!gtk_text_iter_forward_to_tag_toggle (&end, tag))
		{
			g_assert_not_reached ();
		}
		g_assert_true (gtk_text_iter_ends_tag (&end, tag));

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

static void
test_current_word (void)
{
	GtkTextBuffer *buffer;
	GspellInlineCheckerTextBuffer *inline_checker;
	GtkTextIter iter;
	GtkTextIter start;
	GtkTextIter end;

	buffer = create_buffer ();
	inline_checker = _gspell_inline_checker_text_buffer_new (buffer);
	_gspell_inline_checker_text_buffer_set_unit_test_mode (inline_checker, TRUE);

	gtk_text_buffer_get_start_iter (buffer, &iter);
	insert_text_one_char_at_a_time (buffer, &iter, "Hella");
	check_highlighted_words (buffer, inline_checker, -1);

	gtk_text_buffer_insert (buffer, &iter, " ", -1);
	check_highlighted_words (buffer,
				 inline_checker,
				 0, 5,
				 -1);

	gtk_text_buffer_backspace (buffer, &iter, TRUE, TRUE);
	check_highlighted_words (buffer, inline_checker, -1);

	gtk_text_buffer_backspace (buffer, &iter, TRUE, TRUE);
	check_highlighted_words (buffer, inline_checker, -1);

	gtk_text_buffer_insert (buffer, &iter, "o", -1);
	check_highlighted_words (buffer, inline_checker, -1);

	gtk_text_buffer_insert (buffer, &iter, " ", -1);
	check_highlighted_words (buffer, inline_checker, -1);

	insert_text_one_char_at_a_time (buffer, &iter, "nrst");
	check_highlighted_words (buffer, inline_checker, -1);

	/* Cursor movement -> misspelled word highlighted. */
	gtk_text_iter_backward_cursor_position (&iter);
	gtk_text_buffer_place_cursor (buffer, &iter);

	/* Buffer content: "Hello nrs|t".
	 * Cursor position: between 's' and 't'.
	 */
	check_highlighted_words (buffer,
				 inline_checker,
				 6, 10,
				 -1);

	/* Delete the 'e' programmatically, not at the cursor position.
	 * "Hello nrs|t" -> "Hllo nrs|t"
	 */
	gtk_text_buffer_get_iter_at_offset (buffer, &start, 1);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, 2);
	gtk_text_buffer_delete (buffer, &start, &end);
	check_highlighted_words (buffer,
				 inline_checker,
				 0, 4,
				 5, 9, /* "nrst" still highlighted */
				 -1);

	/* Insert 'e' programmatically, not at the cursor position.
	 * "Hllo nrs|t" -> "Hello nrs|t"
	 */
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 1);
	gtk_text_buffer_insert (buffer, &iter, "e", -1);
	check_highlighted_words (buffer,
				 inline_checker,
				 6, 10, /* "nrst" still highlighted */
				 -1);

	/* Delete "nrst" programmatically.
	 * "Hello nrs|t" -> "Hello |"
	 */
	gtk_text_buffer_get_iter_at_offset (buffer, &start, 6);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, 10);
	gtk_text_buffer_delete (buffer, &start, &end);
	check_highlighted_words (buffer, inline_checker, -1);

	/* Insert several chars at once at the cursor position (e.g. a paste).
	 * "Hello |" -> "Hello nrstkw|"
	 */
	gtk_text_buffer_get_end_iter (buffer, &iter);
	gtk_text_buffer_insert (buffer, &iter, "nrstkw", -1);
	check_highlighted_words (buffer,
				 inline_checker,
				 6, 12,
				 -1);

	/* Insert a comma to split a word in two.
	 * "Hello nrs|tkw" -> "Hello nrs,|tkw"
	 */
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 9);
	gtk_text_buffer_place_cursor (buffer, &iter);
	gtk_text_buffer_insert (buffer, &iter, ",", -1);
	check_highlighted_words (buffer,
				 inline_checker,
				 6, 9,
				 10, 13, /* "tkw" also highlighted */
				 -1);

	/* Delete selection.
	 * " H|el|llo " -> " H|llo "
	 */
	gtk_text_buffer_set_text (buffer, " Helllo ", -1);
	gtk_text_buffer_get_iter_at_offset (buffer, &start, 2);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, 4);
	gtk_text_buffer_select_range (buffer, &start, &end);
	gtk_text_buffer_delete (buffer, &start, &end);
	check_highlighted_words (buffer,
				 inline_checker,
				 1, 5,
				 -1);

	/* Backspace at end of word.
	 * " Hllo |" -> " Hllo|"
	 */
	gtk_text_buffer_get_end_iter (buffer, &iter);
	gtk_text_buffer_place_cursor (buffer, &iter);
	gtk_text_buffer_backspace (buffer, &iter, TRUE, TRUE);
	check_highlighted_words (buffer, inline_checker, -1);

	/* Backspace before a word.
	 * " |Hllo" -> "|Hllo"
	 */
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 1);
	gtk_text_buffer_place_cursor (buffer, &iter);
	gtk_text_buffer_backspace (buffer, &iter, TRUE, TRUE);
	check_highlighted_words (buffer,
				 inline_checker,
				 0, 4,
				 -1);

	/* Delete at beginning of word.
	 * "| Hllo " -> "|Hllo "
	 */
	gtk_text_buffer_set_text (buffer, " Hllo ", -1);
	gtk_text_buffer_get_start_iter (buffer, &start);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, 1);
	gtk_text_buffer_place_cursor (buffer, &start);
	gtk_text_buffer_delete (buffer, &start, &end);
	check_highlighted_words (buffer, inline_checker, -1);

	/* Delete after a word.
	 * "Hllo| " -> "Hllo|"
	 */
	gtk_text_buffer_get_iter_at_offset (buffer, &start, 4);
	gtk_text_buffer_get_end_iter (buffer, &end);
	gtk_text_buffer_place_cursor (buffer, &start);
	gtk_text_buffer_delete (buffer, &start, &end);
	check_highlighted_words (buffer,
				 inline_checker,
				 0, 4,
				 -1);

	g_object_unref (inline_checker);
	g_object_unref (buffer);
}

static void
test_apostrophes (void)
{
	GtkTextBuffer *buffer;
	GspellInlineCheckerTextBuffer *inline_checker;
	GtkTextIter iter;
	GtkTextIter start;
	GtkTextIter end;

	buffer = create_buffer ();
	inline_checker = _gspell_inline_checker_text_buffer_new (buffer);
	_gspell_inline_checker_text_buffer_set_unit_test_mode (inline_checker, TRUE);

	gtk_text_buffer_set_text (buffer, "jlvd't ", -1);
	check_highlighted_words (buffer,
				 inline_checker,
				 0, 6,
				 -1);

	/* Delete the 't'.
	 * Note that the apostrophe is no longer highlighted.
	 * This works because we connect to the ::delete-range signal with and
	 * without the AFTER flag, to get the broader word boundaries.
	 */
	gtk_text_buffer_get_iter_at_offset (buffer, &start, 5);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, 6);
	gtk_text_buffer_delete (buffer, &start, &end);
	check_highlighted_words (buffer,
				 inline_checker,
				 0, 4,
				 -1);

	gtk_text_buffer_set_text (buffer, "jlvd't ", -1);
	check_highlighted_words (buffer,
				 inline_checker,
				 0, 6,
				 -1);

	/* Insert a space after the apostrophe.
	 * Note that the apostrophe is no longer highlighted.
	 * This works because we connect to the ::insert-text signal with and
	 * without the AFTER flag, to get the broader word boundaries.
	 */
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, 5);
	gtk_text_buffer_insert (buffer, &iter, " ", -1);
	check_highlighted_words (buffer,
				 inline_checker,
				 0, 4,
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

	g_test_add_func ("/inline-checker-text-buffer/current-word",
			 test_current_word);

	g_test_add_func ("/inline-checker-text-buffer/apostrophes",
			 test_apostrophes);

	return g_test_run ();
}
