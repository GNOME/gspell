/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2015 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef GSPELL_LANGUAGE_CHOOSER_BUTTON_H
#define GSPELL_LANGUAGE_CHOOSER_BUTTON_H

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <gspell/gspell-language.h>
#include <gspell/gspell-version.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_LANGUAGE_CHOOSER_BUTTON (gspell_language_chooser_button_get_type ())

GSPELL_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (GspellLanguageChooserButton, gspell_language_chooser_button,
			  GSPELL, LANGUAGE_CHOOSER_BUTTON,
			  GtkButton)

struct _GspellLanguageChooserButtonClass
{
	GtkButtonClass parent_class;

	/* Padding for future expansion */
	gpointer padding[8];
};

GSPELL_AVAILABLE_IN_ALL
GtkWidget *	gspell_language_chooser_button_new	(const GspellLanguage *current_language);

G_END_DECLS

#endif /* GSPELL_LANGUAGE_CHOOSER_BUTTON_H */

/* ex:set ts=8 noet: */
