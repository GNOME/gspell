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

#ifndef GSPELL_ENTRY_H
#define GSPELL_ENTRY_H

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_ENTRY (gspell_entry_get_type ())

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE (GspellEntry, gspell_entry,
		      GSPELL, ENTRY,
		      GObject)

G_MODULE_EXPORT
GspellEntry *	gspell_entry_get_from_gtk_entry		(GtkEntry *gtk_entry);

G_MODULE_EXPORT
void		gspell_entry_basic_setup		(GspellEntry *gspell_entry);

G_MODULE_EXPORT
GtkEntry *	gspell_entry_get_entry			(GspellEntry *gspell_entry);

G_MODULE_EXPORT
gboolean	gspell_entry_get_inline_spell_checking	(GspellEntry *gspell_entry);

G_MODULE_EXPORT
void		gspell_entry_set_inline_spell_checking	(GspellEntry *gspell_entry,
							 gboolean     enable);

G_END_DECLS

#endif /* GSPELL_ENTRY_H */
