/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2017, 2022 - Sébastien Wilmet
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

#ifndef GSPELL_VERSION_H
#define GSPELL_VERSION_H

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gmodule.h>

G_BEGIN_DECLS

#define GSPELL_AVAILABLE_IN_ALL G_MODULE_EXPORT
#define GSPELL_AVAILABLE_IN_1_2 G_MODULE_EXPORT
#define GSPELL_AVAILABLE_IN_1_4 G_MODULE_EXPORT
#define GSPELL_AVAILABLE_IN_1_6 G_MODULE_EXPORT

G_END_DECLS

#endif /* GSPELL_VERSION_H */
