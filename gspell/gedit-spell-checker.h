/*
 * gedit-spell-checker.h
 * This file is part of gedit
 *
 * Copyright (C) 2002-2006 Paolo Maggi
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

#ifndef __GEDIT_SPELL_CHECKER_H__
#define __GEDIT_SPELL_CHECKER_H__

#include <glib-object.h>
#include "gedit-spell-checker-language.h"

G_BEGIN_DECLS

#define GEDIT_TYPE_SPELL_CHECKER (gedit_spell_checker_get_type ())
G_DECLARE_DERIVABLE_TYPE (GeditSpellChecker, gedit_spell_checker,
			  GEDIT, SPELL_CHECKER,
			  GObject)

/**
 * GEDIT_SPELL_CHECKER_ERROR:
 *
 * Error domain for the spell checker. Errors in this domain will be from the
 * #GeditSpellCheckerError enumeration. See #GError for more information on
 * error domains.
 */
#define GEDIT_SPELL_CHECKER_ERROR (gedit_spell_checker_error_quark ())

/**
 * GeditSpellCheckerError:
 * @GEDIT_SPELL_CHECKER_ERROR_DICTIONARY: dictionary error.
 *
 * An error code used with %GEDIT_SPELL_CHECKER_ERROR in a #GError returned
 * from a spell-checker-related function.
 */
typedef enum _GeditSpellCheckerError GeditSpellCheckerError;
enum _GeditSpellCheckerError
{
	GEDIT_SPELL_CHECKER_ERROR_DICTIONARY,
};

struct _GeditSpellCheckerClass
{
	GObjectClass parent_class;

	/* Signals */
	void (* add_word_to_personal)	(GeditSpellChecker *checker,
					 const gchar       *word);

	void (* add_word_to_session)	(GeditSpellChecker *checker,
					 const gchar       *word);

	void (* clear_session)		(GeditSpellChecker *checker);
};

GQuark		gedit_spell_checker_error_quark			(void);

GeditSpellChecker *
		gedit_spell_checker_new				(const GeditSpellCheckerLanguage *language);

gboolean	gedit_spell_checker_set_language		(GeditSpellChecker               *checker,
								 const GeditSpellCheckerLanguage *language);

const GeditSpellCheckerLanguage *
		gedit_spell_checker_get_language		(GeditSpellChecker *checker);

gboolean	gedit_spell_checker_check_word			(GeditSpellChecker  *checker,
								 const gchar        *word,
								 GError            **error);

GSList *	gedit_spell_checker_get_suggestions		(GeditSpellChecker *checker,
								 const gchar       *word);

void		gedit_spell_checker_add_word_to_personal	(GeditSpellChecker *checker,
								 const gchar       *word);

void		gedit_spell_checker_add_word_to_session		(GeditSpellChecker *checker,
								 const gchar       *word);

void		gedit_spell_checker_clear_session		(GeditSpellChecker *checker);

void		gedit_spell_checker_set_correction		(GeditSpellChecker *checker,
								 const gchar       *word,
								 const gchar       *replacement);

G_END_DECLS

#endif  /* __GEDIT_SPELL_CHECKER_H__ */

/* ex:set ts=8 noet: */
