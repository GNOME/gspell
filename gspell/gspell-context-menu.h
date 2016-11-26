/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2016 - Sébastien Wilmet
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

#ifndef GSPELL_CONTEXT_MENU_H
#define GSPELL_CONTEXT_MENU_H

#include <gtk/gtk.h>
#include "gspell-checker.h"
#include "gspell-language.h"

G_BEGIN_DECLS

typedef void (*GspellLanguageActivatedCallback) (const GspellLanguage *lang,
						 gpointer              user_data);

typedef void (*GspellSuggestionActivatedCallback) (const gchar *suggested_word,
						   gpointer     user_data);

G_GNUC_INTERNAL
GtkMenuItem *	_gspell_context_menu_get_language_menu_item	(const GspellLanguage            *current_language,
								 GspellLanguageActivatedCallback  callback,
								 gpointer                         user_data);

G_GNUC_INTERNAL
GtkMenuItem *	_gspell_context_menu_get_suggestions_menu_item	(GspellChecker                     *checker,
								 const gchar                       *misspelled_word,
								 GspellSuggestionActivatedCallback  callback,
								 gpointer                           user_data);

G_END_DECLS

#endif /* GSPELL_CONTEXT_MENU_H */

/* ex:set ts=8 noet: */
