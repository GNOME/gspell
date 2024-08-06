/* SPDX-FileCopyrightText: 2015 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <gspell/gspell-language.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_LANGUAGE_CHOOSER_BUTTON (gspell_language_chooser_button_get_type ())

G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE (GspellLanguageChooserButton, gspell_language_chooser_button,
			  GSPELL, LANGUAGE_CHOOSER_BUTTON,
			  GtkButton)

struct _GspellLanguageChooserButtonClass
{
	GtkButtonClass parent_class;

	/* Padding for future expansion */
	gpointer padding[8];
};

G_MODULE_EXPORT
GtkWidget *	gspell_language_chooser_button_new	(const GspellLanguage *current_language);

G_END_DECLS
