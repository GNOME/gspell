/* SPDX-FileCopyrightText: 2020 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <glib.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
char *	_gspell_icu_get_language_name_from_code	(const char *language_code,
						 const char *inLocaleID);

/* Intermediate functions, for unit tests: */

G_GNUC_INTERNAL
char *	_gspell_icu_loc_getDisplayNameSimple	(const char *localeID,
						 const char *inLocaleID);

G_END_DECLS
