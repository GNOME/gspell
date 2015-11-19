/*
 * This file is part of gspell.
 *
 * Copyright 2002 - Paolo Maggi
 * Copyright 2015 - Sébastien Wilmet
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GSPELL_LANGUAGE_CHOOSER_DIALOG_H__
#define __GSPELL_LANGUAGE_CHOOSER_DIALOG_H__

#include <gtk/gtk.h>
#include <gspell/gspell-language.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_LANGUAGE_CHOOSER_DIALOG (gspell_language_chooser_dialog_get_type ())
G_DECLARE_DERIVABLE_TYPE (GspellLanguageChooserDialog, gspell_language_chooser_dialog,
			  GSPELL, LANGUAGE_CHOOSER_DIALOG,
			  GtkDialog)

struct _GspellLanguageChooserDialogClass
{
	GtkDialogClass parent_class;

	/* Padding for future expansion */
	gpointer padding[8];
};

GtkWidget *	gspell_language_chooser_dialog_new		(GtkWindow            *parent,
								 const GspellLanguage *current_language);

G_END_DECLS

#endif  /* __GSPELL_LANGUAGE_CHOOSER_DIALOG_H__ */

/* ex:set ts=8 noet: */
