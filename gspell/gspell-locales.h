/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2006 - Paolo Maggi
 * Copyright 2008 - Novell, Inc.
 * Copyright 2015, 2016 - Sébastien Wilmet
 * Copyright 2018 - Chun-wei Fan
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

#ifndef GSPELL_LOCALES_H
#define GSPELL_LOCALES_H

#include <glib-object.h>

G_GNUC_INTERNAL
G_DECLARE_FINAL_TYPE (GspellLocales, gspell_locales,
			  GSPELL, LOCALES,
			  GObject)

#define GSPELL_TYPE_LOCALES (gspell_locales_get_type ())

G_GNUC_INTERNAL GHashTable *
gspell_locales_get_iso_639_names (GspellLocales *locales);

G_GNUC_INTERNAL GHashTable *
gspell_locales_get_iso_3166_names (GspellLocales *locales);

#endif /* GSPELL_LOCALES_H */
