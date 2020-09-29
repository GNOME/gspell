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
#include <unicode/ustring.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
char *			_gspell_icu_strToUTF8			(int32_t     *pDestLength,
								 const UChar *src,
								 int32_t      srcLength,
								 UErrorCode  *pErrorCode);

G_GNUC_INTERNAL
char *			_gspell_icu_strToUTF8Simple		(const UChar *uchars);

G_GNUC_INTERNAL
UChar *			_gspell_icu_loc_getDisplayName		(const char *localeID,
								 const char *inLocaleID,
								 UErrorCode *err);

G_GNUC_INTERNAL
char *			_gspell_icu_loc_getDisplayNameSimple	(const char *localeID,
								 const char *inLocaleID);

G_GNUC_INTERNAL
char *			_gspell_icu_loc_canonicalize		(const char *localeID,
								 UErrorCode *err);

G_GNUC_INTERNAL
char *			_gspell_icu_loc_canonicalizeSimple	(const char *localeID);

G_GNUC_INTERNAL
char *			_gspell_icu_get_language_name_from_code	(const char *language_code);

G_END_DECLS

#endif /* GSPELL_ICU_H */
