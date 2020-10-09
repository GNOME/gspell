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

#ifndef GSPELL_ICU_H
#define GSPELL_ICU_H

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

#endif /* GSPELL_ICU_H */
