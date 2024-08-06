/* SPDX-FileCopyrightText: 2002-2006 - Paolo Maggi
 * SPDX-FileCopyrightText: 2015 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <glib-object.h>
#include <gmodule.h>
#include <enchant.h>
#include <gspell/gspell-language.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_CHECKER (gspell_checker_get_type ())

G_MODULE_EXPORT
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
 * @GSPELL_CHECKER_ERROR_NO_LANGUAGE_SET: no language set.
 *
 * An error code used with %GSPELL_CHECKER_ERROR in a #GError returned
 * from a spell-checker-related function.
 */
typedef enum _GspellCheckerError
{
	GSPELL_CHECKER_ERROR_DICTIONARY,
	GSPELL_CHECKER_ERROR_NO_LANGUAGE_SET,
} GspellCheckerError;

struct _GspellCheckerClass
{
	GObjectClass parent_class;

	/* Signals */
	void (* word_added_to_personal)	(GspellChecker *checker,
					 const gchar   *word);

	void (* word_added_to_session)	(GspellChecker *checker,
					 const gchar   *word);

	void (* session_cleared)	(GspellChecker *checker);

	/* Padding for future expansion */
	gpointer padding[12];
};

G_MODULE_EXPORT
GQuark		gspell_checker_error_quark		(void);

G_MODULE_EXPORT
GspellChecker *	gspell_checker_new			(const GspellLanguage *language);

G_MODULE_EXPORT
void		gspell_checker_set_language		(GspellChecker        *checker,
							 const GspellLanguage *language);

G_MODULE_EXPORT
const GspellLanguage *
		gspell_checker_get_language		(GspellChecker *checker);

G_MODULE_EXPORT
gboolean	gspell_checker_check_word		(GspellChecker  *checker,
							 const gchar    *word,
							 gssize          word_length,
							 GError        **error);

G_MODULE_EXPORT
GSList *	gspell_checker_get_suggestions		(GspellChecker *checker,
							 const gchar   *word,
							 gssize         word_length);

G_MODULE_EXPORT
void		gspell_checker_add_word_to_personal	(GspellChecker *checker,
							 const gchar   *word,
							 gssize         word_length);

G_MODULE_EXPORT
void		gspell_checker_add_word_to_session	(GspellChecker *checker,
							 const gchar   *word,
							 gssize         word_length);

G_MODULE_EXPORT
void		gspell_checker_clear_session		(GspellChecker *checker);

G_MODULE_EXPORT
void		gspell_checker_set_correction		(GspellChecker *checker,
							 const gchar   *word,
							 gssize         word_length,
							 const gchar   *replacement,
							 gssize         replacement_length);

G_MODULE_EXPORT
EnchantDict *	gspell_checker_get_enchant_dict		(GspellChecker *checker);

G_END_DECLS
