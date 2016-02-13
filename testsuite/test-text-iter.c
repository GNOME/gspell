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

#include "gspell/gspell-text-iter.h"

static void
check_forward_word_end (const gchar *text,
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
	ret = _gspell_text_iter_forward_word_end (&iter);
	g_assert_cmpint (ret, ==, expected_ret);

	result_offset = gtk_text_iter_get_offset (&iter);
	g_assert_cmpint (result_offset, ==, expected_result_offset);

	g_object_unref (buffer);
}

static void
test_forward_word_end (void)
{
	check_forward_word_end (" don't ", 0, 6, TRUE);
	check_forward_word_end (" don't ", 1, 6, TRUE);
	check_forward_word_end (" don't ", 2, 6, TRUE);
	check_forward_word_end (" don't ", 4, 6, TRUE);
	check_forward_word_end (" don't ", 5, 6, TRUE);
	check_forward_word_end (" don't ", 6, 6, FALSE);
	check_forward_word_end (" don't", 0, 6, FALSE);

	/* "goin'" is a valid word, but it is not supported yet. */
	check_forward_word_end (" goin' ", 0, 5, TRUE);
	check_forward_word_end (" goin' ", 1, 5, TRUE);
	check_forward_word_end (" goin' ", 2, 5, TRUE);
	check_forward_word_end (" goin' ", 4, 5, TRUE);
	check_forward_word_end (" goin' ", 5, 5, FALSE);
	check_forward_word_end (" goin' ", 6, 6, FALSE);
	check_forward_word_end (" goin' ", 7, 7, FALSE);
}

gint
main (gint    argc,
      gchar **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/text-iter/forward-word-end", test_forward_word_end);

	return g_test_run ();
}
