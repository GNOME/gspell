/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2015, 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef GSPELL_NAVIGATOR_TEXT_VIEW_H
#define GSPELL_NAVIGATOR_TEXT_VIEW_H

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <gspell/gspell-navigator.h>
#include <gspell/gspell-checker.h>
#include <gspell/gspell-version.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_NAVIGATOR_TEXT_VIEW (gspell_navigator_text_view_get_type ())

GSPELL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GspellNavigatorTextView, gspell_navigator_text_view,
			  GSPELL, NAVIGATOR_TEXT_VIEW,
			  GInitiallyUnowned)

struct _GspellNavigatorTextViewClass
{
	GInitiallyUnownedClass parent_class;

	/* Padding for future expansion */
	gpointer padding[8];
};

GSPELL_AVAILABLE_IN_ALL
GspellNavigator *	gspell_navigator_text_view_new		(GtkTextView *view);

GSPELL_AVAILABLE_IN_ALL
GtkTextView *		gspell_navigator_text_view_get_view	(GspellNavigatorTextView *navigator);

G_END_DECLS

#endif /* GSPELL_NAVIGATOR_TEXT_VIEW_H */

/* ex:set ts=8 noet: */
