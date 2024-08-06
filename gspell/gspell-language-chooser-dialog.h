/* SPDX-FileCopyrightText: 2002 - Paolo Maggi
 * SPDX-FileCopyrightText: 2015 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gtk/gtk.h>
#include <gspell/gspell-language.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_LANGUAGE_CHOOSER_DIALOG (gspell_language_chooser_dialog_get_type ())

G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE (GspellLanguageChooserDialog, gspell_language_chooser_dialog,
			  GSPELL, LANGUAGE_CHOOSER_DIALOG,
			  GtkDialog)

struct _GspellLanguageChooserDialogClass
{
	GtkDialogClass parent_class;

	/* Padding for future expansion */
	gpointer padding[8];
};

G_MODULE_EXPORT
GtkWidget *	gspell_language_chooser_dialog_new		(GtkWindow            *parent,
								 const GspellLanguage *current_language,
								 GtkDialogFlags        flags);

G_END_DECLS
