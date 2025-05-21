/* SPDX-FileCopyrightText: 2002-2006 - Paolo Maggi
 * SPDX-FileCopyrightText: 2015, 2016 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell-config.h"
#include "gspell-checker.h"
#include "gspell-checker-private.h"
#include <glib/gi18n-lib.h>
#include <string.h>
#include "gspell-utils.h"

/**
 * SECTION:checker
 * @Short_description: Spell checker
 * @Title: GspellChecker
 * @See_also: #GspellLanguage
 *
 * #GspellChecker is a spell checker.
 *
 * If the #GspellChecker:language property is %NULL, it means that no
 * dictonaries are available, in which case the #GspellChecker is in a
 * “disabled” (but allowed) state.
 *
 * gspell uses the [Enchant](https://rrthomas.github.io/enchant/) library. The
 * use of Enchant is part of the gspell API, #GspellChecker exposes the
 * EnchantDict with the gspell_checker_get_enchant_dict() function.
 */

typedef struct _GspellCheckerPrivate GspellCheckerPrivate;

struct _GspellCheckerPrivate
{
	EnchantBroker *broker;
	EnchantDict *dict;
	const GspellLanguage *active_lang;
};

enum
{
	PROP_0,
	PROP_LANGUAGE,
};

enum
{
	SIGNAL_WORD_ADDED_TO_PERSONAL,
	SIGNAL_WORD_ADDED_TO_SESSION,
	SIGNAL_SESSION_CLEARED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE (GspellChecker, gspell_checker, G_TYPE_OBJECT)

GQuark
gspell_checker_error_quark (void)
{
	static GQuark quark = 0;

	if (G_UNLIKELY (quark == 0))
	{
		quark = g_quark_from_static_string ("gspell-checker-error-quark");
	}

	return quark;
}

static void
gspell_checker_set_property (GObject      *object,
			     guint         prop_id,
			     const GValue *value,
			     GParamSpec   *pspec)
{
	GspellChecker *checker = GSPELL_CHECKER (object);

	switch (prop_id)
	{
		case PROP_LANGUAGE:
			gspell_checker_set_language (checker, g_value_get_boxed (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_checker_get_property (GObject    *object,
			     guint       prop_id,
			     GValue     *value,
			     GParamSpec *pspec)
{
	GspellChecker *checker = GSPELL_CHECKER (object);

	switch (prop_id)
	{
		case PROP_LANGUAGE:
			g_value_set_boxed (value, gspell_checker_get_language (checker));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_checker_finalize (GObject *object)
{
	GspellCheckerPrivate *priv;

	priv = gspell_checker_get_instance_private (GSPELL_CHECKER (object));

	if (priv->dict != NULL)
	{
		enchant_broker_free_dict (priv->broker, priv->dict);
	}

	if (priv->broker != NULL)
	{
		enchant_broker_free (priv->broker);
	}

	G_OBJECT_CLASS (gspell_checker_parent_class)->finalize (object);
}

static void
gspell_checker_class_init (GspellCheckerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = gspell_checker_set_property;
	object_class->get_property = gspell_checker_get_property;
	object_class->finalize = gspell_checker_finalize;

	/**
	 * GspellChecker:language:
	 *
	 * The #GspellLanguage used.
	 */
	g_object_class_install_property (object_class,
					 PROP_LANGUAGE,
					 g_param_spec_boxed ("language",
							     "Language",
							     "",
							     GSPELL_TYPE_LANGUAGE,
							     G_PARAM_READWRITE |
							     G_PARAM_CONSTRUCT |
							     G_PARAM_STATIC_STRINGS));

	/**
	 * GspellChecker::word-added-to-personal:
	 * @spell_checker: the #GspellChecker.
	 * @word: the added word.
	 *
	 * Emitted when a word is added to the personal dictionary.
	 */
	signals[SIGNAL_WORD_ADDED_TO_PERSONAL] =
		g_signal_new ("word-added-to-personal",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GspellCheckerClass, word_added_to_personal),
			      NULL, NULL, NULL,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

	/**
	 * GspellChecker::word-added-to-session:
	 * @spell_checker: the #GspellChecker.
	 * @word: the added word.
	 *
	 * Emitted when a word is added to the session dictionary. See
	 * gspell_checker_add_word_to_session().
	 */
	signals[SIGNAL_WORD_ADDED_TO_SESSION] =
		g_signal_new ("word-added-to-session",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GspellCheckerClass, word_added_to_session),
			      NULL, NULL, NULL,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

	/**
	 * GspellChecker::session-cleared:
	 * @spell_checker: the #GspellChecker.
	 *
	 * Emitted when the session dictionary is cleared.
	 */
	signals[SIGNAL_SESSION_CLEARED] =
		g_signal_new ("session-cleared",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GspellCheckerClass, session_cleared),
			      NULL, NULL, NULL,
			      G_TYPE_NONE,
			      0);
}

static void
gspell_checker_init (GspellChecker *checker)
{
	GspellCheckerPrivate *priv;

	priv = gspell_checker_get_instance_private (checker);

	priv->broker = enchant_broker_init ();
	priv->dict = NULL;
	priv->active_lang = NULL;
}

/**
 * gspell_checker_new:
 * @language: (nullable): the #GspellLanguage to use, or %NULL.
 *
 * Creates a new #GspellChecker. If @language is %NULL, the default language is
 * picked with gspell_language_get_default().
 *
 * Returns: a new #GspellChecker object.
 */
GspellChecker *
gspell_checker_new (const GspellLanguage *language)
{
	return g_object_new (GSPELL_TYPE_CHECKER,
			     "language", language,
			     NULL);
}

static void
create_new_dictionary (GspellChecker *checker)
{
	GspellCheckerPrivate *priv;
	const gchar *language_code;
	const gchar *app_name;

	priv = gspell_checker_get_instance_private (checker);

	if (priv->dict != NULL)
	{
		enchant_broker_free_dict (priv->broker, priv->dict);
		priv->dict = NULL;
	}

	if (priv->active_lang == NULL)
	{
		return;
	}

	language_code = gspell_language_get_code (priv->active_lang);
	priv->dict = enchant_broker_request_dict (priv->broker, language_code);

	if (priv->dict == NULL)
	{
		/* Should never happen, no need to return a GError. */
		g_warning ("Impossible to create an Enchant dictionary for the language code '%s'.",
			   language_code);

		priv->active_lang = NULL;
		return;
	}

	app_name = g_get_application_name ();
	gspell_checker_add_word_to_session (checker, app_name, -1);
}

/* Used for unit tests. Useful to force a NULL language. */
void
_gspell_checker_force_set_language (GspellChecker        *checker,
				    const GspellLanguage *language)
{
	GspellCheckerPrivate *priv;

	g_return_if_fail (GSPELL_IS_CHECKER (checker));

	priv = gspell_checker_get_instance_private (checker);

	if (priv->active_lang != language)
	{
		priv->active_lang = language;
		create_new_dictionary (checker);
		g_object_notify (G_OBJECT (checker), "language");
	}
}

/**
 * gspell_checker_set_language:
 * @checker: a #GspellChecker.
 * @language: (nullable): the #GspellLanguage to use, or %NULL.
 *
 * Sets the language to use for the spell checking. If @language is %NULL, the
 * default language is picked with gspell_language_get_default().
 */
void
gspell_checker_set_language (GspellChecker        *checker,
			     const GspellLanguage *language)
{
	g_return_if_fail (GSPELL_IS_CHECKER (checker));

	if (language == NULL)
	{
		language = gspell_language_get_default ();
	}

	_gspell_checker_force_set_language (checker, language);
}

/**
 * gspell_checker_get_language:
 * @checker: a #GspellChecker.
 *
 * Returns: (nullable): the #GspellLanguage currently used, or %NULL
 * if no dictionaries are available.
 */
const GspellLanguage *
gspell_checker_get_language (GspellChecker *checker)
{
	GspellCheckerPrivate *priv;

	g_return_val_if_fail (GSPELL_IS_CHECKER (checker), NULL);

	priv = gspell_checker_get_instance_private (checker);

	return priv->active_lang;
}

/**
 * gspell_checker_check_word:
 * @checker: a #GspellChecker.
 * @word: the word to check.
 * @word_length: the byte length of @word, or -1 if @word is nul-terminated.
 * @error: (out) (optional): a location to a %NULL #GError, or %NULL.
 *
 * If the #GspellChecker:language is %NULL, i.e. when no dictonaries are
 * available, this function returns %TRUE to limit the damage.
 *
 * Returns: %TRUE if @word is correctly spelled, %FALSE otherwise.
 */
gboolean
gspell_checker_check_word (GspellChecker  *checker,
			   const gchar    *word,
			   gssize          word_length,
			   GError        **error)
{
	GspellCheckerPrivate *priv;
	gint enchant_result;
	gboolean correctly_spelled;
	gchar *sanitized_word;

	g_return_val_if_fail (GSPELL_IS_CHECKER (checker), FALSE);
	g_return_val_if_fail (word != NULL, FALSE);
	g_return_val_if_fail (word_length >= -1, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	priv = gspell_checker_get_instance_private (checker);

	if (priv->dict == NULL)
	{
		return TRUE;
	}

	if (_gspell_utils_is_number (word, word_length))
	{
		return TRUE;
	}

	if (_gspell_utils_str_to_ascii_apostrophe (word, word_length, &sanitized_word))
	{
		enchant_result = enchant_dict_check (priv->dict, sanitized_word, -1);
		g_free (sanitized_word);
	}
	else
	{
		enchant_result = enchant_dict_check (priv->dict, word, word_length);
	}

	correctly_spelled = enchant_result == 0;

	if (enchant_result < 0)
	{
		gchar *nul_terminated_word;

		if (word_length == -1)
		{
			word_length = strlen (word);
		}

		nul_terminated_word = g_strndup (word, word_length);

		g_set_error (error,
			     GSPELL_CHECKER_ERROR,
			     GSPELL_CHECKER_ERROR_DICTIONARY,
			     _("Error when checking the spelling of word “%s”: %s"),
			     nul_terminated_word,
			     enchant_dict_get_error (priv->dict));

		g_free (nul_terminated_word);
	}

	return correctly_spelled;
}

/**
 * gspell_checker_get_suggestions:
 * @checker: a #GspellChecker.
 * @word: a misspelled word.
 * @word_length: the byte length of @word, or -1 if @word is nul-terminated.
 *
 * Gets the suggestions for @word. Free the return value with
 * g_slist_free_full(suggestions, g_free).
 *
 * Returns: (transfer full) (element-type utf8): the list of suggestions.
 */
GSList *
gspell_checker_get_suggestions (GspellChecker *checker,
				const gchar   *word,
				gssize         word_length)
{
	GspellCheckerPrivate *priv;
	gchar *sanitized_word;
	gchar **suggestions;
	GSList *suggestions_list = NULL;
	gint i;

	g_return_val_if_fail (GSPELL_IS_CHECKER (checker), NULL);
	g_return_val_if_fail (word != NULL, NULL);
	g_return_val_if_fail (word_length >= -1, NULL);

	priv = gspell_checker_get_instance_private (checker);

	if (priv->dict == NULL)
	{
		return NULL;
	}

	if (_gspell_utils_str_to_ascii_apostrophe (word, word_length, &sanitized_word))
	{
		suggestions = enchant_dict_suggest (priv->dict, sanitized_word, -1, NULL);
		g_free (sanitized_word);
	}
	else
	{
		suggestions = enchant_dict_suggest (priv->dict, word, word_length, NULL);
	}

	if (suggestions == NULL)
	{
		return NULL;
	}

	for (i = 0; suggestions[i] != NULL; i++)
	{
		suggestions_list = g_slist_prepend (suggestions_list, suggestions[i]);
	}

	/* The array/list elements will be freed by the caller. */
	g_free (suggestions);

	return g_slist_reverse (suggestions_list);
}

/**
 * gspell_checker_add_word_to_personal:
 * @checker: a #GspellChecker.
 * @word: a word.
 * @word_length: the byte length of @word, or -1 if @word is nul-terminated.
 *
 * Adds a word to the personal dictionary. It is typically saved in the user's
 * home directory.
 */
void
gspell_checker_add_word_to_personal (GspellChecker *checker,
				     const gchar   *word,
				     gssize         word_length)
{
	GspellCheckerPrivate *priv;

	g_return_if_fail (GSPELL_IS_CHECKER (checker));
	g_return_if_fail (word != NULL);
	g_return_if_fail (word_length >= -1);

	priv = gspell_checker_get_instance_private (checker);

	if (priv->dict == NULL)
	{
		return;
	}

	enchant_dict_add (priv->dict, word, word_length);

	if (word_length == -1)
	{
		g_signal_emit (G_OBJECT (checker),
			       signals[SIGNAL_WORD_ADDED_TO_PERSONAL], 0,
			       word);
	}
	else
	{
		gchar *nul_terminated_word = g_strndup (word, word_length);

		g_signal_emit (G_OBJECT (checker),
			       signals[SIGNAL_WORD_ADDED_TO_PERSONAL], 0,
			       nul_terminated_word);

		g_free (nul_terminated_word);
	}
}

/**
 * gspell_checker_add_word_to_session:
 * @checker: a #GspellChecker.
 * @word: a word.
 * @word_length: the byte length of @word, or -1 if @word is nul-terminated.
 *
 * Adds a word to the session dictionary. Each #GspellChecker instance has a
 * different session dictionary. The session dictionary is lost when the
 * #GspellChecker:language property changes or when @checker is destroyed or
 * when gspell_checker_clear_session() is called.
 *
 * This function is typically called for an “Ignore All” action.
 */
void
gspell_checker_add_word_to_session (GspellChecker *checker,
				    const gchar   *word,
				    gssize         word_length)
{
	GspellCheckerPrivate *priv;

	g_return_if_fail (GSPELL_IS_CHECKER (checker));
	g_return_if_fail (word != NULL);
	g_return_if_fail (word_length >= -1);

	priv = gspell_checker_get_instance_private (checker);

	if (priv->dict == NULL)
	{
		return;
	}

	enchant_dict_add_to_session (priv->dict, word, word_length);

	if (word_length == -1)
	{
		g_signal_emit (G_OBJECT (checker),
			       signals[SIGNAL_WORD_ADDED_TO_SESSION], 0,
			       word);
	}
	else
	{
		gchar *nul_terminated_word = g_strndup (word, word_length);

		g_signal_emit (G_OBJECT (checker),
			       signals[SIGNAL_WORD_ADDED_TO_SESSION], 0,
			       nul_terminated_word);

		g_free (nul_terminated_word);
	}
}

/**
 * gspell_checker_clear_session:
 * @checker: a #GspellChecker.
 *
 * Clears the session dictionary.
 */
void
gspell_checker_clear_session (GspellChecker *checker)
{
	g_return_if_fail (GSPELL_IS_CHECKER (checker));

	/* Free and re-request dictionary. */
	create_new_dictionary (checker);

	g_signal_emit (G_OBJECT (checker), signals[SIGNAL_SESSION_CLEARED], 0);
}

/**
 * gspell_checker_set_correction:
 * @checker: a #GspellChecker.
 * @word: a word.
 * @word_length: the byte length of @word, or -1 if @word is nul-terminated.
 * @replacement: the replacement word.
 * @replacement_length: the byte length of @replacement, or -1 if @replacement
 *   is nul-terminated.
 *
 * Informs the spell checker that @word is replaced/corrected by @replacement.
 */
void
gspell_checker_set_correction (GspellChecker *checker,
			       const gchar   *word,
			       gssize         word_length,
			       const gchar   *replacement,
			       gssize         replacement_length)
{
	GspellCheckerPrivate *priv;

	g_return_if_fail (GSPELL_IS_CHECKER (checker));
	g_return_if_fail (word != NULL);
	g_return_if_fail (word_length >= -1);
	g_return_if_fail (replacement != NULL);
	g_return_if_fail (replacement_length >= -1);

	priv = gspell_checker_get_instance_private (checker);

	if (priv->dict == NULL)
	{
		return;
	}

	enchant_dict_store_replacement (priv->dict,
					word, word_length,
					replacement, replacement_length);
}

/**
 * gspell_checker_get_enchant_dict: (skip)
 * @checker: a #GspellChecker.
 *
 * Gets the EnchantDict currently used by @checker. It permits to extend
 * #GspellChecker with more features. Note that by doing so, the other classes
 * in gspell may no longer work well.
 *
 * #GspellChecker re-creates a new EnchantDict when the #GspellChecker:language
 * is changed and when the session is cleared.
 *
 * Returns: (transfer none) (nullable): the EnchantDict currently used by
 * @checker.
 * Since: 1.6
 */
EnchantDict *
gspell_checker_get_enchant_dict (GspellChecker *checker)
{
	GspellCheckerPrivate *priv;

	g_return_val_if_fail (GSPELL_IS_CHECKER (checker), NULL);

	priv = gspell_checker_get_instance_private (checker);
	return priv->dict;
}
