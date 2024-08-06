/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
