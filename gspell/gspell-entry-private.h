/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2016 - Sébastien Wilmet
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

#ifndef GSPELL_ENTRY_PRIVATE_H
#define GSPELL_ENTRY_PRIVATE_H

#include "gspell/gspell-entry.h"

G_BEGIN_DECLS

G_GNUC_INTERNAL
const GSList *	_gspell_entry_get_misspelled_words	(GspellEntry *gspell_entry);

G_END_DECLS

#endif /* GSPELL_ENTRY_PRIVATE_H */

/* ex:set ts=8 noet: */
