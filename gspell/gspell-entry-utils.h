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

#ifndef GSPELL_ENTRY_UTILS_H
#define GSPELL_ENTRY_UTILS_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _GspellEntryWord GspellEntryWord;
struct _GspellEntryWord
{
	gchar *word_str;

	/* Position in the GtkEntryBuffer. The character at the byte_end index
	 * is not included, like in #PangoAttribute.
	 */
	gint byte_start;
	gint byte_end;

	/* The same as @byte_start and @byte_end, but in characters.
	 * Useful for example for the #GtkEditable functions.
	 */
	gint char_start;
	gint char_end;
};

G_GNUC_INTERNAL
GspellEntryWord *_gspell_entry_word_new				(void);

G_GNUC_INTERNAL
void		 _gspell_entry_word_free			(gpointer data);

G_GNUC_INTERNAL
GSList *	 _gspell_entry_utils_get_words			(GtkEntry *entry);

G_GNUC_INTERNAL
gint		 _gspell_entry_utils_get_char_position_at_event	(GtkEntry       *entry,
								 GdkEventButton *event);

G_END_DECLS

#endif /* GSPELL_ENTRY_UTILS_H */

/* ex:set ts=8 noet: */
