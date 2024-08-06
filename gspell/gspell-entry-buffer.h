/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gspell/gspell-checker.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_ENTRY_BUFFER (gspell_entry_buffer_get_type ())

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE (GspellEntryBuffer, gspell_entry_buffer,
		      GSPELL, ENTRY_BUFFER,
		      GObject)

G_MODULE_EXPORT
GspellEntryBuffer *	gspell_entry_buffer_get_from_gtk_entry_buffer	(GtkEntryBuffer *gtk_buffer);

G_MODULE_EXPORT
GtkEntryBuffer *	gspell_entry_buffer_get_buffer			(GspellEntryBuffer *gspell_buffer);

G_MODULE_EXPORT
GspellChecker *		gspell_entry_buffer_get_spell_checker		(GspellEntryBuffer *gspell_buffer);

G_MODULE_EXPORT
void			gspell_entry_buffer_set_spell_checker		(GspellEntryBuffer *gspell_buffer,
									 GspellChecker     *spell_checker);

G_END_DECLS
