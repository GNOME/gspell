/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include <gspell/gspell.h>
#include "gspell/gspell-utils.h"

static void
test_check_word (void)
{
	const GspellLanguage *lang;
	GspellChecker *checker;
	gboolean correctly_spelled;
	GError *error = NULL;

	lang = gspell_language_lookup ("en_US");
	g_assert_true (lang != NULL);

	checker = gspell_checker_new (lang);

	correctly_spelled = gspell_checker_check_word (checker, "hello", -1, &error);
	g_assert_no_error (error);
	g_assert_true (correctly_spelled);

	correctly_spelled = gspell_checker_check_word (checker, "tkbqzat", -1, &error);
	g_assert_no_error (error);
	g_assert_true (!correctly_spelled);

	g_object_unref (checker);
}

static void
test_apostrophes (void)
{
	const GspellLanguage *lang;
	GspellChecker *checker;
	gboolean correctly_spelled;
	gunichar apostrophe_char;
	GError *error = NULL;

	lang = gspell_language_lookup ("en_US");
	g_assert_true (lang != NULL);

	checker = gspell_checker_new (lang);

	/* Apostrophe U+0027 */

	apostrophe_char = g_utf8_get_char ("'");
	g_assert_cmpint (apostrophe_char, ==, '\'');

	correctly_spelled = gspell_checker_check_word (checker, "doesn't", -1, &error);
	g_assert_no_error (error);
	g_assert_true (correctly_spelled);

	/* Modifier Letter Apostrophe U+02BC */

	apostrophe_char = g_utf8_get_char ("\xCA\xBC");
	g_assert_cmpint (apostrophe_char, ==, _GSPELL_MODIFIER_LETTER_APOSTROPHE);

	correctly_spelled = gspell_checker_check_word (checker, "doesn\xCA\xBCt", -1, &error);
	g_assert_no_error (error);
	g_assert_true (correctly_spelled);

	/* Right Single Quotation Mark U+2019 */

	apostrophe_char = g_utf8_get_char ("\xE2\x80\x99");
	g_assert_cmpint (apostrophe_char, ==, _GSPELL_RIGHT_SINGLE_QUOTATION_MARK);

	correctly_spelled = gspell_checker_check_word (checker, "doesn\xE2\x80\x99t", -1, &error);
	g_assert_no_error (error);
	g_assert_true (correctly_spelled);

	g_object_unref (checker);
}

static void
test_dashes (void)
{
	const GspellLanguage *lang;
	GspellChecker *checker;
	gboolean correctly_spelled;
	GError *error = NULL;

	lang = gspell_language_lookup ("en_US");
	g_assert_true (lang != NULL);

	checker = gspell_checker_new (lang);

	/* This unit test fails with the aspell enchant backend, but aspell can
	 * be considered deprecated, it is better to use hunspell, so WONTFIX.
	 * For more details, see:
	 * https://bugzilla.gnome.org/show_bug.cgi?id=772406
	 */
	correctly_spelled = gspell_checker_check_word (checker, "spell-checking", -1, &error);
	g_assert_no_error (error);
	g_assert_true (correctly_spelled);

	correctly_spelled = gspell_checker_check_word (checker, "nrst-auie", -1, &error);
	g_assert_no_error (error);
	g_assert_true (!correctly_spelled);

	g_object_unref (checker);
}

gint
main (gint    argc,
      gchar **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/checker/check_word", test_check_word);
	g_test_add_func ("/checker/apostrophes", test_apostrophes);
	g_test_add_func ("/checker/dashes", test_dashes);

	return g_test_run ();
}
