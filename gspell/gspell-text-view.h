/* SPDX-FileCopyrightText: 2015, 2016 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_TEXT_VIEW (gspell_text_view_get_type ())

G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE (GspellTextView, gspell_text_view,
			  GSPELL, TEXT_VIEW,
			  GObject)

struct _GspellTextViewClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	gpointer padding[8];
};

G_MODULE_EXPORT
GspellTextView *	gspell_text_view_get_from_gtk_text_view		(GtkTextView *gtk_view);

G_MODULE_EXPORT
void			gspell_text_view_basic_setup			(GspellTextView *gspell_view);

G_MODULE_EXPORT
GtkTextView *		gspell_text_view_get_view			(GspellTextView *gspell_view);

G_MODULE_EXPORT
gboolean		gspell_text_view_get_inline_spell_checking	(GspellTextView *gspell_view);

G_MODULE_EXPORT
void			gspell_text_view_set_inline_spell_checking	(GspellTextView *gspell_view,
									 gboolean        enable);

G_MODULE_EXPORT
gboolean		gspell_text_view_get_enable_language_menu	(GspellTextView *gspell_view);

G_MODULE_EXPORT
void			gspell_text_view_set_enable_language_menu	(GspellTextView *gspell_view,
									 gboolean        enable_language_menu);

G_END_DECLS
