/*
 * This file is part of gspell.
 *
 * Copyright 2002 - Paolo Maggi
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

#ifndef __GSPELL_SPELL_LANGUAGE_DIALOG_H__
#define __GSPELL_SPELL_LANGUAGE_DIALOG_H__

#include <gtk/gtk.h>
#include "gspell-spell-checker-language.h"

G_BEGIN_DECLS

#define GSPELL_TYPE_SPELL_LANGUAGE_DIALOG (gspell_spell_language_dialog_get_type ())
G_DECLARE_FINAL_TYPE (GspellSpellLanguageDialog, gspell_spell_language_dialog,
		      GSPELL, SPELL_LANGUAGE_DIALOG,
		      GtkDialog)

GtkWidget *	gspell_spell_language_dialog_new				(GtkWindow			 *parent,
									  const GspellSpellCheckerLanguage *cur_lang);

const GspellSpellCheckerLanguage *
		gspell_spell_language_dialog_get_selected_language	(GspellSpellLanguageDialog *dialog);

G_END_DECLS

#endif  /* __GSPELL_SPELL_LANGUAGE_DIALOG_H__ */

/* ex:set ts=8 noet: */
