/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2020 - Sébastien Wilmet <swilmet@gnome.org>
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

#include <gtk/gtk.h>
#include "gspell/gspell-icu.h"

static void
check_loc_getDisplayNameSimple (const char *localeID,
				const char *expected_display_name)
{
	char *received_display_name;

	received_display_name = _gspell_icu_loc_getDisplayNameSimple (localeID, "en_US");
	g_assert_cmpstr (received_display_name, ==, expected_display_name);
	g_free (received_display_name);
}

static void
test_loc_getDisplayNameSimple (void)
{
	check_loc_getDisplayNameSimple ("en_US", "English (United States)");
}

static void
check_get_language_name_from_code (const char *language_code,
				   const char *expected_name)
{
	char *received_name;

	received_name = _gspell_icu_get_language_name_from_code (language_code, "en_US");
	g_assert_cmpstr (received_name, ==, expected_name);
	g_free (received_name);
}

static void
test_get_language_name_from_code (void)
{
	check_get_language_name_from_code ("en_US", "English (United States)");
}

int
main (int    argc,
      char **argv)
{
	gtk_test_init (&argc, &argv);

	g_test_add_func ("/icu/loc_getDisplayNameSimple", test_loc_getDisplayNameSimple);
	g_test_add_func ("/icu/get_language_name_from_code", test_get_language_name_from_code);

	return g_test_run ();
}
