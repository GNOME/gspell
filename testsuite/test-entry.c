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

#include "gspell/gspell-entry-utils.h"

static GSList *
add_word (GSList      *list,
	  const gchar *word_str,
	  gint         byte_start,
	  gint         byte_end)
{
	GspellEntryWord *word;

	word = _gspell_entry_word_new ();
	word->word_str = g_strdup (word_str);
	word->byte_start = byte_start;
	word->byte_end = byte_end;

	return g_slist_append (list, word);
}

static void
free_word_list (GSList *list)
{
	g_slist_free_full (list, _gspell_entry_word_free);
}

static void
check_entry_word_equal (GspellEntryWord *word1,
			GspellEntryWord *word2)
{
	g_assert_cmpstr (word1->word_str, ==, word2->word_str);
	g_assert_cmpint (word1->byte_start, ==, word2->byte_start);
	g_assert_cmpint (word1->byte_end, ==, word2->byte_end);
}

static void
check_entry_word_list_equal (GSList *list1,
			     GSList *list2)
{
	GSList *l1;
	GSList *l2;

	for (l1 = list1, l2 = list2;
	     l1 != NULL && l2 != NULL;
	     l1 = l1->next, l2 = l2->next)
	{
		GspellEntryWord *word1 = l1->data;
		GspellEntryWord *word2 = l2->data;

		check_entry_word_equal (word1, word2);
	}

	g_assert (l1 == NULL);
	g_assert (l2 == NULL);
}

static void
test_get_words (void)
{
	GtkEntry *entry;
	GSList *expected_list;
	GSList *received_list;

	entry = GTK_ENTRY (gtk_entry_new ());
	g_object_ref_sink (entry);

	expected_list = NULL;
	received_list = _gspell_entry_utils_get_words (entry);
	check_entry_word_list_equal (expected_list, received_list);

	/* Only one word */
	gtk_entry_set_text (entry, "Finntroll");
	expected_list = add_word (NULL, "Finntroll", 0, 9);
	received_list = _gspell_entry_utils_get_words (entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);
	free_word_list (received_list);

	/* Only one word, not at the start and end */
	gtk_entry_set_text (entry, " Finntroll ");
	expected_list = add_word (NULL, "Finntroll", 1, 10);
	received_list = _gspell_entry_utils_get_words (entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);
	free_word_list (received_list);

	/* Several words */
	gtk_entry_set_text (entry, "Finntroll - Svart Djup");
	expected_list = add_word (NULL, "Finntroll", 0, 9);
	expected_list = add_word (expected_list, "Svart", 12, 17);
	expected_list = add_word (expected_list, "Djup", 18, 22);
	received_list = _gspell_entry_utils_get_words (entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);
	free_word_list (received_list);

	/* Multi-byte UTF-8 words */
	// å takes two bytes.
	// ö takes two bytes.
	gtk_entry_set_text (entry, "Asfågelns Död");
	expected_list = add_word (NULL, "Asfågelns", 0, 10);
	expected_list = add_word (expected_list, "Död", 11, 15);
	received_list = _gspell_entry_utils_get_words (entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);
	free_word_list (received_list);

	g_object_unref (entry);
}

gint
main (gint    argc,
      gchar **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/entry-utils/get-words", test_get_words);

	return g_test_run ();
}
