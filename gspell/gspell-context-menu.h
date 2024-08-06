/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

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
