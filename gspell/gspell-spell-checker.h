/*
 * This file is part of gspell.
 *
 * Copyright 2002-2006 - Paolo Maggi
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

#ifndef __GSPELL_SPELL_CHECKER_H__
#define __GSPELL_SPELL_CHECKER_H__

#include <glib-object.h>
#include "gspell-spell-checker-language.h"

G_BEGIN_DECLS

#define GSPELL_TYPE_SPELL_CHECKER (gspell_spell_checker_get_type ())
G_DECLARE_DERIVABLE_TYPE (GspellSpellChecker, gspell_spell_checker,
			  GSPELL, SPELL_CHECKER,
			  GObject)

/**
 * GSPELL_SPELL_CHECKER_ERROR:
 *
 * Error domain for the spell checker. Errors in this domain will be from the
 * #GspellSpellCheckerError enumeration. See #GError for more information on
 * error domains.
 */
#define GSPELL_SPELL_CHECKER_ERROR (gspell_spell_checker_error_quark ())

/**
 * GspellSpellCheckerError:
 * @GSPELL_SPELL_CHECKER_ERROR_DICTIONARY: dictionary error.
 *
 * An error code used with %GSPELL_SPELL_CHECKER_ERROR in a #GError returned
 * from a spell-checker-related function.
 */
typedef enum _GspellSpellCheckerError GspellSpellCheckerError;
enum _GspellSpellCheckerError
{
	GSPELL_SPELL_CHECKER_ERROR_DICTIONARY,
};

struct _GspellSpellCheckerClass
{
	GObjectClass parent_class;

	/* Signals */
	void (* add_word_to_personal)	(GspellSpellChecker *checker,
					 const gchar       *word);

	void (* add_word_to_session)	(GspellSpellChecker *checker,
					 const gchar       *word);

	void (* clear_session)		(GspellSpellChecker *checker);
};

GQuark		gspell_spell_checker_error_quark			(void);

GspellSpellChecker *
		gspell_spell_checker_new				(const GspellSpellCheckerLanguage *language);

gboolean	gspell_spell_checker_set_language		(GspellSpellChecker               *checker,
								  const GspellSpellCheckerLanguage *language);

const GspellSpellCheckerLanguage *
		gspell_spell_checker_get_language		(GspellSpellChecker *checker);

gboolean	gspell_spell_checker_check_word			(GspellSpellChecker  *checker,
								  const gchar        *word,
								  GError            **error);

GSList *	gspell_spell_checker_get_suggestions		(GspellSpellChecker *checker,
								  const gchar       *word);

void		gspell_spell_checker_add_word_to_personal	(GspellSpellChecker *checker,
								  const gchar       *word);

void		gspell_spell_checker_add_word_to_session		(GspellSpellChecker *checker,
								  const gchar       *word);

void		gspell_spell_checker_clear_session		(GspellSpellChecker *checker);

void		gspell_spell_checker_set_correction		(GspellSpellChecker *checker,
								  const gchar       *word,
								  const gchar       *replacement);

G_END_DECLS

#endif  /* __GSPELL_SPELL_CHECKER_H__ */

/* ex:set ts=8 noet: */
