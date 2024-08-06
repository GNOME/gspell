/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

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
