/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

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
