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

#ifndef __GSPELL_CHECKER_H__
#define __GSPELL_CHECKER_H__

#include <glib-object.h>
#include "gspell-language.h"

G_BEGIN_DECLS

#define GSPELL_TYPE_CHECKER (gspell_checker_get_type ())
G_DECLARE_DERIVABLE_TYPE (GspellChecker, gspell_checker,
			  GSPELL, CHECKER,
			  GObject)

/**
 * GSPELL_CHECKER_ERROR:
 *
 * Error domain for the spell checker. Errors in this domain will be from the
 * #GspellCheckerError enumeration. See #GError for more information on
 * error domains.
 */
#define GSPELL_CHECKER_ERROR (gspell_checker_error_quark ())

/**
 * GspellCheckerError:
 * @GSPELL_CHECKER_ERROR_DICTIONARY: dictionary error.
 *
 * An error code used with %GSPELL_CHECKER_ERROR in a #GError returned
 * from a spell-checker-related function.
 */
typedef enum _GspellCheckerError GspellCheckerError;
enum _GspellCheckerError
{
	GSPELL_CHECKER_ERROR_DICTIONARY,
};

struct _GspellCheckerClass
{
	GObjectClass parent_class;

	/* Signals */
	void (* add_word_to_personal)	(GspellChecker *checker,
					 const gchar       *word);

	void (* add_word_to_session)	(GspellChecker *checker,
					 const gchar       *word);

	void (* clear_session)		(GspellChecker *checker);
};

GQuark		gspell_checker_error_quark			(void);

GspellChecker *
		gspell_checker_new				(const GspellLanguage *language);

gboolean	gspell_checker_set_language		(GspellChecker               *checker,
								  const GspellLanguage *language);

const GspellLanguage *
		gspell_checker_get_language		(GspellChecker *checker);

gboolean	gspell_checker_check_word			(GspellChecker  *checker,
								  const gchar        *word,
								  GError            **error);

GSList *	gspell_checker_get_suggestions		(GspellChecker *checker,
								  const gchar       *word);

void		gspell_checker_add_word_to_personal	(GspellChecker *checker,
								  const gchar       *word);

void		gspell_checker_add_word_to_session		(GspellChecker *checker,
								  const gchar       *word);

void		gspell_checker_clear_session		(GspellChecker *checker);

void		gspell_checker_set_correction		(GspellChecker *checker,
								  const gchar       *word,
								  const gchar       *replacement);

G_END_DECLS

#endif  /* __GSPELL_CHECKER_H__ */

/* ex:set ts=8 noet: */
