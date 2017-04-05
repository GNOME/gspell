/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2015, 2016 - Sébastien Wilmet
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

#ifndef GSPELL_TEXT_VIEW_H
#define GSPELL_TEXT_VIEW_H

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <gspell/gspell-version.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_TEXT_VIEW (gspell_text_view_get_type ())

GSPELL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GspellTextView, gspell_text_view,
			  GSPELL, TEXT_VIEW,
			  GObject)

struct _GspellTextViewClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	gpointer padding[8];
};

GSPELL_AVAILABLE_IN_ALL
GspellTextView *	gspell_text_view_get_from_gtk_text_view		(GtkTextView *gtk_view);

GSPELL_AVAILABLE_IN_1_2
void			gspell_text_view_basic_setup			(GspellTextView *gspell_view);

GSPELL_AVAILABLE_IN_ALL
GtkTextView *		gspell_text_view_get_view			(GspellTextView *gspell_view);

GSPELL_AVAILABLE_IN_ALL
gboolean		gspell_text_view_get_inline_spell_checking	(GspellTextView *gspell_view);

GSPELL_AVAILABLE_IN_ALL
void			gspell_text_view_set_inline_spell_checking	(GspellTextView *gspell_view,
									 gboolean        enable);

GSPELL_AVAILABLE_IN_1_2
gboolean		gspell_text_view_get_enable_language_menu	(GspellTextView *gspell_view);

GSPELL_AVAILABLE_IN_1_2
void			gspell_text_view_set_enable_language_menu	(GspellTextView *gspell_view,
									 gboolean        enable_language_menu);

G_END_DECLS

#endif /* GSPELL_TEXT_VIEW_H */

/* ex:set ts=8 noet: */
