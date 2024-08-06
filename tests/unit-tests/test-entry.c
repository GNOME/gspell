/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <gspell/gspell.h>
#include "gspell/gspell-entry-utils.h"
#include "gspell/gspell-entry-private.h"
#include "gspell/gspell-checker-private.h"

/* Returns: (transfer full) */
static GtkEntry *
create_entry (void)
{
	GtkEntry *gtk_entry;
	GtkEntryBuffer *gtk_buffer;
	const GspellLanguage *lang;
	GspellChecker *checker;
	GspellEntryBuffer *gspell_buffer;
	GspellEntry *gspell_entry;

	gtk_entry = GTK_ENTRY (gtk_entry_new ());
	g_object_ref_sink (gtk_entry);

	lang = gspell_language_lookup ("en_US");
	g_assert_true (lang != NULL);

	checker = gspell_checker_new (lang);
	gtk_buffer = gtk_entry_get_buffer (gtk_entry);
	gspell_buffer = gspell_entry_buffer_get_from_gtk_entry_buffer (gtk_buffer);
	gspell_entry_buffer_set_spell_checker (gspell_buffer, checker);
	g_object_unref (checker);

	gspell_entry = gspell_entry_get_from_gtk_entry (gtk_entry);
	gspell_entry_set_inline_spell_checking (gspell_entry, TRUE);

	return gtk_entry;
}

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
	word->char_start = -1;
	word->char_end = -1;

	return g_slist_append (list, word);
}

static GSList *
add_word_full (GSList      *list,
	       const gchar *word_str,
	       gint         byte_start,
	       gint         byte_end,
	       gint         char_start,
	       gint         char_end)
{
	GspellEntryWord *word;

	word = _gspell_entry_word_new ();
	word->word_str = g_strdup (word_str);
	word->byte_start = byte_start;
	word->byte_end = byte_end;
	word->char_start = char_start;
	word->char_end = char_end;

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
	g_assert_true (word1 != NULL);
	g_assert_true (word2 != NULL);

	g_assert_cmpstr (word1->word_str, ==, word2->word_str);
	g_assert_cmpint (word1->byte_start, ==, word2->byte_start);
	g_assert_cmpint (word1->byte_end, ==, word2->byte_end);

	if (word1->char_start != -1 &&
	    word2->char_start != -1)
	{
		g_assert_cmpint (word1->char_start, ==, word2->char_start);
	}

	if (word1->char_end != -1 &&
	    word2->char_end != -1)
	{
		g_assert_cmpint (word1->char_end, ==, word2->char_end);
	}
}

static void
check_entry_word_list_equal (const GSList *list1,
			     const GSList *list2)
{
	const GSList *l1;
	const GSList *l2;

	for (l1 = list1, l2 = list2;
	     l1 != NULL && l2 != NULL;
	     l1 = l1->next, l2 = l2->next)
	{
		GspellEntryWord *word1 = l1->data;
		GspellEntryWord *word2 = l2->data;

		check_entry_word_equal (word1, word2);
	}

	g_assert_true (l1 == NULL);
	g_assert_true (l2 == NULL);
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
	expected_list = add_word_full (NULL, "Finntroll", 0, 9, 0, 9);
	received_list = _gspell_entry_utils_get_words (entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);
	free_word_list (received_list);

	/* Only one word, not at the start and end */
	gtk_entry_set_text (entry, " Finntroll ");
	expected_list = add_word_full (NULL, "Finntroll", 1, 10, 1, 10);
	received_list = _gspell_entry_utils_get_words (entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);
	free_word_list (received_list);

	/* Several words */
	gtk_entry_set_text (entry, "Finntroll - Svart Djup");
	expected_list = add_word_full (NULL, "Finntroll", 0, 9, 0, 9);
	expected_list = add_word_full (expected_list, "Svart", 12, 17, 12, 17);
	expected_list = add_word_full (expected_list, "Djup", 18, 22, 18, 22);
	received_list = _gspell_entry_utils_get_words (entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);
	free_word_list (received_list);

	/* Multi-byte UTF-8 words */
	// å takes two bytes.
	// ö takes two bytes.
	gtk_entry_set_text (entry, "Asfågelns Död");
	expected_list = add_word_full (NULL, "Asfågelns", 0, 10, 0, 9);
	expected_list = add_word_full (expected_list, "Död", 11, 15, 10, 13);
	received_list = _gspell_entry_utils_get_words (entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);
	free_word_list (received_list);

	/* One apostrophe */
	gtk_entry_set_text (entry, "doesn't");
	expected_list = add_word_full (NULL, "doesn't", 0, 7, 0, 7);
	received_list = _gspell_entry_utils_get_words (entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);
	free_word_list (received_list);

	/* Several apostrophes */
	gtk_entry_set_text (entry, "rock'n'roll");
	expected_list = add_word_full (NULL, "rock'n'roll", 0, 11, 0, 11);
	received_list = _gspell_entry_utils_get_words (entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);
	free_word_list (received_list);

	/* Apostrophe at end of word. Not yet supported. */
	gtk_entry_set_text (entry, "doin'");
	expected_list = add_word_full (NULL, "doin", 0, 4, 0, 4);
	received_list = _gspell_entry_utils_get_words (entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);
	free_word_list (received_list);

	/* Apostrophe at beginning of word. Not yet supported. */
	gtk_entry_set_text (entry, "'til");
	expected_list = add_word_full (NULL, "til", 1, 4, 1, 4);
	received_list = _gspell_entry_utils_get_words (entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);
	free_word_list (received_list);

	/* Dash */
	gtk_entry_set_text (entry, "spell-checking");
	expected_list = add_word_full (NULL, "spell-checking", 0, 14, 0, 14);
	received_list = _gspell_entry_utils_get_words (entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);
	free_word_list (received_list);

	g_object_unref (entry);
}

static void
test_init (void)
{
	GtkEntry *gtk_entry;
	GtkEntryBuffer *gtk_buffer;
	const GspellLanguage *lang;
	GspellChecker *checker;
	GspellEntryBuffer *gspell_buffer;
	GspellEntry *gspell_entry;
	GSList *expected_list;
	const GSList *received_list;

	gtk_entry = GTK_ENTRY (gtk_entry_new ());
	g_object_ref_sink (gtk_entry);

	/* Set initial text before initializing GspellEntry. */
	gtk_entry_set_text (gtk_entry, "auienrst");

	lang = gspell_language_lookup ("en_US");
	g_assert_true (lang != NULL);

	checker = gspell_checker_new (lang);
	gtk_buffer = gtk_entry_get_buffer (gtk_entry);
	gspell_buffer = gspell_entry_buffer_get_from_gtk_entry_buffer (gtk_buffer);
	gspell_entry_buffer_set_spell_checker (gspell_buffer, checker);
	g_object_unref (checker);

	/* Test just after creating the GspellEntry.
	 * The inline-spell-checking property is FALSE by default, so there are
	 * no misspelled words.
	 */
	gspell_entry = gspell_entry_get_from_gtk_entry (gtk_entry);
	expected_list = NULL;
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);

	gspell_entry_set_inline_spell_checking (gspell_entry, TRUE);
	expected_list = add_word (NULL, "auienrst", 0, 8);
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);

	g_object_unref (gtk_entry);
}

static void
test_inline_spell_checking_property (void)
{
	GtkEntry *gtk_entry;
	GspellEntry *gspell_entry;
	GSList *expected_list;
	const GSList *received_list;

	gtk_entry = create_entry ();
	gspell_entry = gspell_entry_get_from_gtk_entry (gtk_entry);

	expected_list = NULL;
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);

	gspell_entry_set_inline_spell_checking (gspell_entry, FALSE);
	expected_list = NULL;
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);

	gtk_entry_set_text (gtk_entry, "auienrst");
	expected_list = NULL;
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);

	gspell_entry_set_inline_spell_checking (gspell_entry, TRUE);
	expected_list = add_word (NULL, "auienrst", 0, 8);
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);

	gtk_entry_set_text (gtk_entry, "Hello Död");
	expected_list = add_word (NULL, "Död", 6, 10);
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);

	gspell_entry_set_inline_spell_checking (gspell_entry, FALSE);
	expected_list = NULL;
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);

	g_object_unref (gtk_entry);
}

static void
test_buffer_change (void)
{
	GtkEntry *gtk_entry;
	GtkEntryBuffer *other_buffer;
	GspellEntry *gspell_entry;
	GSList *expected_list;
	const GSList *received_list;

	gtk_entry = create_entry ();
	gspell_entry = gspell_entry_get_from_gtk_entry (gtk_entry);

	gtk_entry_set_text (gtk_entry, "auienrst");
	expected_list = add_word (NULL, "auienrst", 0, 8);
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);

	other_buffer = gtk_entry_buffer_new (NULL, -1);
	gtk_entry_set_buffer (gtk_entry, other_buffer);
	g_object_unref (other_buffer);
	expected_list = NULL;
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);

	g_object_unref (gtk_entry);
}

static void
test_spell_checker_change (void)
{
	GtkEntry *gtk_entry;
	GspellEntry *gspell_entry;
	gint i;

	gtk_entry = GTK_ENTRY (gtk_entry_new ());
	g_object_ref_sink (gtk_entry);

	gspell_entry = gspell_entry_get_from_gtk_entry (gtk_entry);
	gspell_entry_set_inline_spell_checking (gspell_entry, TRUE);

	for (i = 0; i < 2; i++)
	{
		GtkEntryBuffer *gtk_buffer;
		GspellEntryBuffer *gspell_buffer;
		GSList *expected_list;
		const GSList *received_list;
		const GspellLanguage *lang;
		GspellChecker *checker;

		gtk_buffer = gtk_entry_get_buffer (gtk_entry);
		gspell_buffer = gspell_entry_buffer_get_from_gtk_entry_buffer (gtk_buffer);

		/* Not yet a spell checker */
		gtk_entry_set_text (gtk_entry, "auienrst");
		expected_list = NULL;
		received_list = _gspell_entry_get_misspelled_words (gspell_entry);
		check_entry_word_list_equal (expected_list, received_list);

		/* Set a spell checker */
		lang = gspell_language_lookup ("en_US");
		g_assert_true (lang != NULL);

		checker = gspell_checker_new (lang);
		gspell_entry_buffer_set_spell_checker (gspell_buffer, checker);
		g_object_unref (checker);

		expected_list = add_word (NULL, "auienrst", 0, 8);
		received_list = _gspell_entry_get_misspelled_words (gspell_entry);
		check_entry_word_list_equal (expected_list, received_list);
		free_word_list (expected_list);

		/* Set NULL spell checker */
		gspell_entry_buffer_set_spell_checker (gspell_buffer, NULL);
		expected_list = NULL;
		received_list = _gspell_entry_get_misspelled_words (gspell_entry);
		check_entry_word_list_equal (expected_list, received_list);

		/* Change buffer for next iteration, to see if the signal
		 * connection for spell-checker changes still works.
		 */
		{
			GtkEntryBuffer *new_gtk_buffer;

			new_gtk_buffer = gtk_entry_buffer_new (NULL, -1);
			gtk_entry_set_buffer (gtk_entry, new_gtk_buffer);
			g_object_unref (new_gtk_buffer);
		}
	}

	g_object_unref (gtk_entry);
}

static void
test_language_change (void)
{
	GtkEntry *gtk_entry;
	GspellEntry *gspell_entry;
	GtkEntryBuffer *gtk_buffer;
	GspellEntryBuffer *gspell_buffer;
	const GspellLanguage *lang;
	GspellChecker *checker;
	GSList *expected_list;
	const GSList *received_list;

	gtk_entry = GTK_ENTRY (gtk_entry_new ());
	g_object_ref_sink (gtk_entry);

	gspell_entry = gspell_entry_get_from_gtk_entry (gtk_entry);
	gspell_entry_set_inline_spell_checking (gspell_entry, TRUE);

	gtk_buffer = gtk_entry_get_buffer (gtk_entry);
	gspell_buffer = gspell_entry_buffer_get_from_gtk_entry_buffer (gtk_buffer);

	lang = gspell_language_lookup ("en_US");
	g_assert_true (lang != NULL);

	checker = gspell_checker_new (lang);
	gspell_entry_buffer_set_spell_checker (gspell_buffer, checker);

	gtk_entry_set_text (gtk_entry, "auienrst");
	expected_list = add_word (NULL, "auienrst", 0, 8);
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);

	_gspell_checker_force_set_language (checker, NULL);
	expected_list = NULL;
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);

	g_object_unref (gtk_entry);
	g_object_unref (checker);
}

static void
test_password_mode (void)
{
	GtkEntry *gtk_entry;
	GspellEntry *gspell_entry;
	GSList *expected_list;
	const GSList *received_list;

	gtk_entry = create_entry ();
	gspell_entry = gspell_entry_get_from_gtk_entry (gtk_entry);

	g_assert_true (gtk_entry_get_visibility (gtk_entry));

	gtk_entry_set_text (gtk_entry, "auienrst");
	expected_list = add_word (NULL, "auienrst", 0, 8);
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);

	gtk_entry_set_visibility (gtk_entry, FALSE);
	expected_list = NULL;
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);

	gtk_entry_set_visibility (gtk_entry, TRUE);
	expected_list = add_word (NULL, "auienrst", 0, 8);
	received_list = _gspell_entry_get_misspelled_words (gspell_entry);
	check_entry_word_list_equal (expected_list, received_list);
	free_word_list (expected_list);

	g_object_unref (gtk_entry);
}

gint
main (gint    argc,
      gchar **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/entry-utils/get-words", test_get_words);
	g_test_add_func ("/entry/init", test_init);
	g_test_add_func ("/entry/inline-spell-checking-property", test_inline_spell_checking_property);
	g_test_add_func ("/entry/buffer-change", test_buffer_change);
	g_test_add_func ("/entry/spell-checker-change", test_spell_checker_change);
	g_test_add_func ("/entry/language-change", test_language_change);
	g_test_add_func ("/entry/password-mode", test_password_mode);

	return g_test_run ();
}
