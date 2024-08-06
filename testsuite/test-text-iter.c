/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell/gspell-text-iter.h"

static void
check_word_move (gboolean     forward,
		 const gchar *text,
		 gint         initial_offset,
		 gint         expected_result_offset,
		 gboolean     expected_ret)
{
	GtkTextBuffer *buffer;
	GtkTextIter iter;
	gboolean ret;
	gint result_offset;

	buffer = gtk_text_buffer_new (NULL);
	gtk_text_buffer_set_text (buffer, text, -1);

	gtk_text_buffer_get_iter_at_offset (buffer, &iter, initial_offset);

	if (forward)
	{
		ret = _gspell_text_iter_forward_word_end (&iter);
	}
	else
	{
		ret = _gspell_text_iter_backward_word_start (&iter);
	}

	g_assert_cmpint (ret, ==, expected_ret);

	result_offset = gtk_text_iter_get_offset (&iter);
	g_assert_cmpint (result_offset, ==, expected_result_offset);

	g_object_unref (buffer);
}

static void
test_forward_word_end (void)
{
	gint i;

	for (i = 0; i <= 5; i++)
	{
		check_word_move (TRUE, " don't ", i, 6, TRUE);
	}
	check_word_move (TRUE, " don't ", 6, 6, FALSE);
	check_word_move (TRUE, " don't", 0, 6, FALSE);

	/* "goin'" is a valid word, but not yet supported. */
	for (i = 0; i <= 4; i++)
	{
		check_word_move (TRUE, " goin' ", i, 5, TRUE);
	}
	check_word_move (TRUE, " goin' ", 5, 5, FALSE);
	check_word_move (TRUE, " goin' ", 6, 6, FALSE);
	check_word_move (TRUE, " goin' ", 7, 7, FALSE);

	/* Several apostrophes in the same word. */
	for (i = 0; i <= 11; i++)
	{
		check_word_move (TRUE, " rock'n'roll ", i, 12, TRUE);
	}
	check_word_move (TRUE, " rock'n'roll ", 12, 12, FALSE);
	check_word_move (TRUE, " rock'n'roll", 0, 12, FALSE);

	/* Dashes */
	for (i = 0; i <= 5; i++)
	{
		check_word_move (TRUE, " as-is ", i, 6, TRUE);
	}
	check_word_move (TRUE, " as-is ", 6, 6, FALSE);
	check_word_move (TRUE, " as-is", 0, 6, FALSE);

	/* In "as-", the word boundaries should be around "as". */
	for (i = 0; i <= 2; i++)
	{
		check_word_move (TRUE, " as- ", i, 3, TRUE);
	}
	check_word_move (TRUE, " as- ", 3, 3, FALSE);
	check_word_move (TRUE, " as- ", 4, 4, FALSE);
	check_word_move (TRUE, " as- ", 5, 5, FALSE);

	/* Several dashes in the same word. */
	for (i = 0; i <= 11; i++)
	{
		check_word_move (TRUE, " rock-n-roll ", i, 12, TRUE);
	}
	check_word_move (TRUE, " rock-n-roll ", 12, 12, FALSE);
	check_word_move (TRUE, " rock-n-roll", 0, 12, FALSE);
}

static void
test_backward_word_start (void)
{
	gint i;

	for (i = 7; i >= 2; i--)
	{
		check_word_move (FALSE, " don't ", i, 1, TRUE);
	}
	check_word_move (FALSE, " don't ", 1, 1, FALSE);
	check_word_move (FALSE, " don't ", 0, 0, FALSE);

	/* "'til" is a valid word, but not yet supported. */
	for (i = 6; i >= 3; i--)
	{
		check_word_move (FALSE, " 'til ", i, 2, TRUE);
	}
	check_word_move (FALSE, " 'til ", 2, 2, FALSE);
	check_word_move (FALSE, " 'til ", 1, 1, FALSE);
	check_word_move (FALSE, " 'til ", 0, 0, FALSE);

	/* Several apostrophes in the same word. */
	for (i = 13; i >= 2; i--)
	{
		check_word_move (FALSE, " rock'n'roll ", i, 1, TRUE);
	}
	check_word_move (FALSE, " rock'n'roll ", 1, 1, FALSE);
	check_word_move (FALSE, " rock'n'roll ", 0, 0, FALSE);

	/* Dashes */
	for (i = 7; i >= 2; i--)
	{
		check_word_move (FALSE, " as-is ", i, 1, TRUE);
	}
	check_word_move (FALSE, " as-is ", 1, 1, FALSE);
	check_word_move (FALSE, " as-is ", 0, 0, FALSE);

	/* In "-as", the word boundaries should be around "as". */
	for (i = 5; i >= 3; i--)
	{
		check_word_move (FALSE, " -as ", i, 2, TRUE);
	}
	check_word_move (FALSE, " -as ", 2, 2, FALSE);
	check_word_move (FALSE, " -as ", 1, 1, FALSE);
	check_word_move (FALSE, " -as ", 0, 0, FALSE);

	/* Several dashes in the same word. */
	for (i = 13; i >= 2; i--)
	{
		check_word_move (FALSE, " rock-n-roll ", i, 1, TRUE);
	}
	check_word_move (FALSE, " rock-n-roll ", 1, 1, FALSE);
	check_word_move (FALSE, " rock-n-roll ", 0, 0, FALSE);
}

static void
check_properties (GtkTextBuffer *buffer,
		  gint           offset,
		  gboolean       expected_starts_word,
		  gboolean       expected_ends_word,
		  gboolean       expected_inside_word)
{
	GtkTextIter iter;

	gtk_text_buffer_get_iter_at_offset (buffer, &iter, offset);

	g_assert_cmpint (expected_starts_word, ==, _gspell_text_iter_starts_word (&iter));
	g_assert_cmpint (expected_ends_word, ==, _gspell_text_iter_ends_word (&iter));
	g_assert_cmpint (expected_inside_word, ==, _gspell_text_iter_inside_word (&iter));
}

static void
test_properties (void)
{
	GtkTextBuffer *buffer;
	gint i;

	buffer = gtk_text_buffer_new (NULL);

	gtk_text_buffer_set_text (buffer, " don't ", -1);
	check_properties (buffer, 0, FALSE, FALSE, FALSE);
	check_properties (buffer, 1, TRUE, FALSE, TRUE);
	for (i = 2; i <= 5; i++)
	{
		check_properties (buffer, i, FALSE, FALSE, TRUE);
	}
	check_properties (buffer, 6, FALSE, TRUE, FALSE);
	check_properties (buffer, 7, FALSE, FALSE, FALSE);

	/* "goin'" is a valid word, but not yet supported. */
	gtk_text_buffer_set_text (buffer, " goin' ", -1);
	check_properties (buffer, 0, FALSE, FALSE, FALSE);
	check_properties (buffer, 1, TRUE, FALSE, TRUE);
	for (i = 2; i <= 4; i++)
	{
		check_properties (buffer, i, FALSE, FALSE, TRUE);
	}
	check_properties (buffer, 5, FALSE, TRUE, FALSE);
	check_properties (buffer, 6, FALSE, FALSE, FALSE);
	check_properties (buffer, 7, FALSE, FALSE, FALSE);

	/* "'til" is a valid word, but not yet supported. */
	gtk_text_buffer_set_text (buffer, " 'til ", -1);
	check_properties (buffer, 0, FALSE, FALSE, FALSE);
	check_properties (buffer, 1, FALSE, FALSE, FALSE);
	check_properties (buffer, 2, TRUE, FALSE, TRUE);
	check_properties (buffer, 3, FALSE, FALSE, TRUE);
	check_properties (buffer, 4, FALSE, FALSE, TRUE);
	check_properties (buffer, 5, FALSE, TRUE, FALSE);
	check_properties (buffer, 6, FALSE, FALSE, FALSE);

	/* Several apostrophes in the same word. */
	gtk_text_buffer_set_text (buffer, " rock'n'roll ", -1);
	check_properties (buffer, 0, FALSE, FALSE, FALSE);
	check_properties (buffer, 1, TRUE, FALSE, TRUE);
	for (i = 2; i <= 11; i++)
	{
		check_properties (buffer, i, FALSE, FALSE, TRUE);
	}
	check_properties (buffer, 12, FALSE, TRUE, FALSE);
	check_properties (buffer, 13, FALSE, FALSE, FALSE);

	g_object_unref (buffer);
}

gint
main (gint    argc,
      gchar **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/text-iter/forward-word-end", test_forward_word_end);
	g_test_add_func ("/text-iter/backward-word-start", test_backward_word_start);
	g_test_add_func ("/text-iter/starts-ends-inside-word", test_properties);

	return g_test_run ();
}
