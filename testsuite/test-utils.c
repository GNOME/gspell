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

#include "gspell/gspell-utils.h"

static void
test_is_number (void)
{
	gboolean is_number;

	is_number = _gspell_utils_is_number ("123", -1);
	g_assert (is_number);

	is_number = _gspell_utils_is_number ("123", 3);
	g_assert (is_number);

	is_number = _gspell_utils_is_number ("123a4", 3);
	g_assert (is_number);

	is_number = _gspell_utils_is_number ("123a4", -1);
	g_assert (!is_number);

	is_number = _gspell_utils_is_number ("h4ck1ng", -1);
	g_assert (!is_number);
}

static void
test_str_replace (void)
{
	gchar *result;

	result = _gspell_utils_str_replace ("$filename", "$filename", "blah");
	g_assert_cmpstr (result, ==, "blah");
	g_free (result);

	result = _gspell_utils_str_replace ("$shortname.pdf", "$shortname", "blah");
	g_assert_cmpstr (result, ==, "blah.pdf");
	g_free (result);

	result = _gspell_utils_str_replace ("abcdabcd", "ab", "r");
	g_assert_cmpstr (result, ==, "rcdrcd");
	g_free (result);

	result = _gspell_utils_str_replace ("abcd", "ef", "r");
	g_assert_cmpstr (result, ==, "abcd");
	g_free (result);
}

static void
test_str_to_ascii_apostrophe (void)
{
	gchar *result = NULL;
	gboolean ret;

	ret = _gspell_utils_str_to_ascii_apostrophe ("rock'n'roll", -1, &result);
	g_assert (!ret);
	g_assert (result == NULL);

	ret = _gspell_utils_str_to_ascii_apostrophe ("rock\xCA\xBCn\xCA\xBCroll", -1, &result);
	g_assert (ret);
	g_assert_cmpstr (result, ==, "rock'n'roll");
	g_free (result);
	result = NULL;

	ret = _gspell_utils_str_to_ascii_apostrophe ("rock\xE2\x80\x99n\xE2\x80\x99roll", -1, &result);
	g_assert (ret);
	g_assert_cmpstr (result, ==, "rock'n'roll");
	g_free (result);
	result = NULL;

	ret = _gspell_utils_str_to_ascii_apostrophe ("rock\xCA\xBCn\xE2\x80\x99roll", -1, &result);
	g_assert (ret);
	g_assert_cmpstr (result, ==, "rock'n'roll");
	g_free (result);
	result = NULL;

	ret = _gspell_utils_str_to_ascii_apostrophe ("rock\xCA\xBCn\xE2\x80\x99roll", 7, &result);
	g_assert (ret);
	g_assert_cmpstr (result, ==, "rock'n");
	g_free (result);
	result = NULL;

	ret = _gspell_utils_str_to_ascii_apostrophe ("rock\xCA\xBCn\xE2\x80\x99roll", 4, &result);
	g_assert (!ret);
	g_assert (result == NULL);
}

gint
main (gint    argc,
      gchar **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/utils/is_number", test_is_number);
	g_test_add_func ("/utils/str_replace", test_str_replace);
	g_test_add_func ("/utils/str_to_ascii_apostrophe", test_str_to_ascii_apostrophe);

	return g_test_run ();
}
