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

static void
test_check_word (void)
{
	const GspellLanguage *lang;
	GspellChecker *checker;
	gboolean correctly_spelled;
	GError *error = NULL;

	lang = gspell_language_lookup ("en_US");
	g_assert (lang != NULL);

	checker = gspell_checker_new (lang);

	correctly_spelled = gspell_checker_check_word (checker, "hello", -1, &error);
	g_assert_no_error (error);
	g_assert_true (correctly_spelled);

	correctly_spelled = gspell_checker_check_word (checker, "tkbqzat", -1, &error);
	g_assert_no_error (error);
	g_assert_false (correctly_spelled);
}

gint
main (gint    argc,
      gchar **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/checker/check_word", test_check_word);

	return g_test_run ();
}
