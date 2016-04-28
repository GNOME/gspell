/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2016 - Ignacio Casal Quinteiro <icq@gnome.org>
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

#ifndef GSPELL_INIT_H
#define GSPELL_INIT_H

#include <glib.h>

#ifdef G_OS_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

G_GNUC_INTERNAL
HMODULE _gspell_init_get_dll (void);

#endif /* G_OS_WIN32 */

#endif /* GSPELL_INIT_H */

/* ex:set ts=8 noet: */
