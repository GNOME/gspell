/* SPDX-FileCopyrightText: 2015, 2016 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gspell/gspell-checker.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_TEXT_BUFFER (gspell_text_buffer_get_type ())

G_MODULE_EXPORT
G_DECLARE_FINAL_TYPE (GspellTextBuffer, gspell_text_buffer,
		      GSPELL, TEXT_BUFFER,
		      GObject)

G_MODULE_EXPORT
GspellTextBuffer *	gspell_text_buffer_get_from_gtk_text_buffer	(GtkTextBuffer *gtk_buffer);

G_MODULE_EXPORT
GtkTextBuffer *		gspell_text_buffer_get_buffer			(GspellTextBuffer *gspell_buffer);

G_MODULE_EXPORT
GspellChecker *		gspell_text_buffer_get_spell_checker		(GspellTextBuffer *gspell_buffer);

G_MODULE_EXPORT
void			gspell_text_buffer_set_spell_checker		(GspellTextBuffer *gspell_buffer,
									 GspellChecker    *spell_checker);

G_END_DECLS
