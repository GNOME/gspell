/* SPDX-FileCopyrightText: 2015, 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <gspell/gspell-navigator.h>
#include <gspell/gspell-checker.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_NAVIGATOR_TEXT_VIEW (gspell_navigator_text_view_get_type ())

G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE (GspellNavigatorTextView, gspell_navigator_text_view,
			  GSPELL, NAVIGATOR_TEXT_VIEW,
			  GInitiallyUnowned)

struct _GspellNavigatorTextViewClass
{
	GInitiallyUnownedClass parent_class;

	/* Padding for future expansion */
	gpointer padding[8];
};

G_MODULE_EXPORT
GspellNavigator *	gspell_navigator_text_view_new		(GtkTextView *view);

G_MODULE_EXPORT
GtkTextView *		gspell_navigator_text_view_get_view	(GspellNavigatorTextView *navigator);

G_END_DECLS
