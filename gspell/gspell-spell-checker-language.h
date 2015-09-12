/*
 * This file is part of gspell.
 *
 * Copyright 2006 - Paolo Maggi
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

#ifndef __GSPELL_SPELL_CHECKER_LANGUAGE_H__
#define __GSPELL_SPELL_CHECKER_LANGUAGE_H__

#include <glib.h>

G_BEGIN_DECLS

typedef struct _GspellSpellCheckerLanguage GspellSpellCheckerLanguage;

const gchar *	gspell_spell_checker_language_to_string		(const GspellSpellCheckerLanguage *lang);

const gchar *	gspell_spell_checker_language_to_key		(const GspellSpellCheckerLanguage *lang);

const GspellSpellCheckerLanguage *
		gspell_spell_checker_language_from_key		(const gchar *key);

/* GSList contains "GspellSpellCheckerLanguage*" items */
const GSList *	gspell_spell_checker_get_available_languages	(void);

G_END_DECLS

#endif /* __GSPELL_SPELL_CHECKER_LANGUAGE_H__ */

/* ex:set ts=8 noet: */
