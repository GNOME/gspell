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

#ifndef GSPELL_ENTRY_BUFFER_H
#define GSPELL_ENTRY_BUFFER_H

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gspell/gspell-checker.h>
#include <gspell/gspell-version.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_ENTRY_BUFFER (gspell_entry_buffer_get_type ())

GSPELL_AVAILABLE_IN_1_4
G_DECLARE_FINAL_TYPE (GspellEntryBuffer, gspell_entry_buffer,
		      GSPELL, ENTRY_BUFFER,
		      GObject)

GSPELL_AVAILABLE_IN_1_4
GspellEntryBuffer *	gspell_entry_buffer_get_from_gtk_entry_buffer	(GtkEntryBuffer *gtk_buffer);

GSPELL_AVAILABLE_IN_1_4
GtkEntryBuffer *	gspell_entry_buffer_get_buffer			(GspellEntryBuffer *gspell_buffer);

GSPELL_AVAILABLE_IN_1_4
GspellChecker *		gspell_entry_buffer_get_spell_checker		(GspellEntryBuffer *gspell_buffer);

GSPELL_AVAILABLE_IN_1_4
void			gspell_entry_buffer_set_spell_checker		(GspellEntryBuffer *gspell_buffer,
									 GspellChecker     *spell_checker);

G_END_DECLS

#endif /* GSPELL_ENTRY_BUFFER_H */

/* ex:set ts=8 noet: */
