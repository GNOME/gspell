/*
 * This file is part of gspell.
 *
 * Copyright 2011, 2014 - Jesse van den Kieboom
 * Copyright 2015 - Sébastien Wilmet
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _GSPELL_OSX_H
#define _GSPELL_OSX_H

#include <glib.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
gchar *	_gspell_osx_get_preferred_spell_language	(void);

G_GNUC_INTERNAL
gchar *	_gspell_osx_get_resource_path			(void);

G_END_DECLS

#endif /* _GSPELL_OSX_H */

/* ex:set ts=8 noet: */
