/*
 * gedit-spell-checker.c
 * This file is part of gedit
 *
 * Copyright (C) 2002-2006 Paolo Maggi
 * Copyright (C) 2015 Sébastien Wilmet
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "gedit-spell-checker.h"
#include <enchant.h>
#include <glib/gi18n.h>
#include "gedit-spell-utils.h"

#ifdef OS_OSX
#include "gedit-spell-osx.h"
#endif

/**
 * SECTION:spell-checker
 * @Short_description: Spell checker
 * @Title: GeditSpellChecker
 * @See_also: #GeditSpellCheckerLanguage
 *
 * #GeditSpellChecker is a spell checker. It is a wrapper around the Enchant
 * library, to have a #GObject-based API.
 *
 * If the #GeditSpellChecker:language is not set correctly, the spell checker
 * should not be used. It can happen when no dictionaries are available, in
 * which case gedit_spell_checker_get_language() returns %NULL.
 */

typedef struct _GeditSpellCheckerPrivate GeditSpellCheckerPrivate;

struct _GeditSpellCheckerPrivate
{
	EnchantBroker *broker;
	EnchantDict *dict;
	const GeditSpellCheckerLanguage *active_lang;
};

enum
{
	PROP_0,
	PROP_LANGUAGE,
};

enum
{
	SIGNAL_ADD_WORD_TO_PERSONAL,
	SIGNAL_ADD_WORD_TO_SESSION,
	SIGNAL_CLEAR_SESSION,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE_WITH_PRIVATE (GeditSpellChecker, gedit_spell_checker, G_TYPE_OBJECT)

GQuark
gedit_spell_checker_error_quark (void)
{
	static GQuark quark = 0;

	if (G_UNLIKELY (quark == 0))
	{
		quark = g_quark_from_static_string ("gedit-spell-checker-error-quark");
	}

	return quark;
}

static gboolean
is_language_set (GeditSpellChecker *checker)
{
	GeditSpellCheckerPrivate *priv;

	priv = gedit_spell_checker_get_instance_private (checker);

	g_assert ((priv->active_lang == NULL && priv->dict == NULL) ||
		  (priv->active_lang != NULL && priv->dict != NULL));

	if (priv->active_lang == NULL)
	{
		g_warning ("Spell checker: the language is not correctly set.\n"
			   "There is maybe no dictionaries available.\n"
			   "Check the return value of gedit_spell_checker_get_language().");
		return FALSE;
	}

	return TRUE;
}

static void
gedit_spell_checker_set_property (GObject      *object,
				  guint         prop_id,
				  const GValue *value,
				  GParamSpec   *pspec)
{
	GeditSpellChecker *checker = GEDIT_SPELL_CHECKER (object);

	switch (prop_id)
	{
		case PROP_LANGUAGE:
			gedit_spell_checker_set_language (checker, g_value_get_pointer (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_spell_checker_get_property (GObject    *object,
				  guint       prop_id,
				  GValue     *value,
				  GParamSpec *pspec)
{
	GeditSpellCheckerPrivate *priv;

	priv = gedit_spell_checker_get_instance_private (GEDIT_SPELL_CHECKER (object));

	switch (prop_id)
	{
		case PROP_LANGUAGE:
			g_value_set_pointer (value, (gpointer)priv->active_lang);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gedit_spell_checker_finalize (GObject *object)
{
	GeditSpellCheckerPrivate *priv;

	priv = gedit_spell_checker_get_instance_private (GEDIT_SPELL_CHECKER (object));

	if (priv->dict != NULL)
	{
		enchant_broker_free_dict (priv->broker, priv->dict);
	}

	if (priv->broker != NULL)
	{
		enchant_broker_free (priv->broker);
	}

	G_OBJECT_CLASS (gedit_spell_checker_parent_class)->finalize (object);
}

static void
gedit_spell_checker_class_init (GeditSpellCheckerClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->set_property = gedit_spell_checker_set_property;
	object_class->get_property = gedit_spell_checker_get_property;
	object_class->finalize = gedit_spell_checker_finalize;

	/**
	 * GeditSpellChecker:language:
	 *
	 * The #GeditSpellCheckerLanguage used.
	 */
	g_object_class_install_property (object_class,
					 PROP_LANGUAGE,
					 g_param_spec_pointer ("language",
							       "Language",
							       "The language used by the spell checker",
							       G_PARAM_READWRITE |
							       G_PARAM_CONSTRUCT |
							       G_PARAM_STATIC_STRINGS));

	/**
	 * GeditSpellChecker::add-word-to-personal:
	 * @spell_checker: the #GeditSpellChecker.
	 * @word: the added word.
	 *
	 * Emitted when a word is added to the personal dictionary.
	 */
	signals[SIGNAL_ADD_WORD_TO_PERSONAL] =
		g_signal_new ("add-word-to-personal",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditSpellCheckerClass, add_word_to_personal),
			      NULL, NULL, NULL,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

	/**
	 * GeditSpellChecker::add-word-to-session:
	 * @spell_checker: the #GeditSpellChecker.
	 * @word: the added word.
	 *
	 * Emitted when a word is added to the session dictionary. The session
	 * dictionary is lost when the application exits.
	 */
	signals[SIGNAL_ADD_WORD_TO_SESSION] =
		g_signal_new ("add-word-to-session",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditSpellCheckerClass, add_word_to_session),
			      NULL, NULL, NULL,
			      G_TYPE_NONE,
			      1,
			      G_TYPE_STRING);

	/**
	 * GeditSpellChecker::clear-session:
	 * @spell_checker: the #GeditSpellChecker.
	 *
	 * Emitted when the session dictionary is cleared.
	 */
	signals[SIGNAL_CLEAR_SESSION] =
		g_signal_new ("clear-session",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (GeditSpellCheckerClass, clear_session),
			      NULL, NULL, NULL,
			      G_TYPE_NONE,
			      0);
}

static void
gedit_spell_checker_init (GeditSpellChecker *checker)
{
	GeditSpellCheckerPrivate *priv;

	priv = gedit_spell_checker_get_instance_private (checker);

	priv->broker = enchant_broker_init ();
	priv->dict = NULL;
	priv->active_lang = NULL;
}

/**
 * gedit_spell_checker_new:
 * @language: (nullable): the #GeditSpellCheckerLanguage to use.
 *
 * Returns: a new #GeditSpellChecker object.
 */
GeditSpellChecker *
gedit_spell_checker_new	(const GeditSpellCheckerLanguage *language)
{
	return g_object_new (GEDIT_TYPE_SPELL_CHECKER,
			     "language", language,
			     NULL);
}

static const GeditSpellCheckerLanguage *
get_default_language (void)
{
	const GeditSpellCheckerLanguage *lang;
	const gchar * const *lang_names;
	const GSList *langs;
	gint i;

	/* Try with the current locale */
	lang_names = g_get_language_names ();

	for (i = 0; lang_names[i] != NULL; i++)
	{
		lang = gedit_spell_checker_language_from_key (lang_names[i]);

		if (lang != NULL)
		{
			return lang;
		}
	}

	/* Another try specific to Mac OS X */
#ifdef OS_OSX
	{
		gchar *key = gedit_spell_osx_get_preferred_spell_language ();

		if (key != NULL)
		{
			lang = gedit_spell_checker_language_from_key (key);
			g_free (key);
			return lang;
		}
	}
#endif

	/* Try English */
	lang = gedit_spell_checker_language_from_key ("en_US");
	if (lang != NULL)
	{
		return lang;
	}

	/* Take the first available language */
	langs = gedit_spell_checker_get_available_languages ();
	if (langs != NULL)
	{
		return langs->data;
	}

	return NULL;
}

static gboolean
init_dictionary (GeditSpellChecker *checker)
{
	GeditSpellCheckerPrivate *priv;
	const gchar *app_name;

	priv = gedit_spell_checker_get_instance_private (checker);

	g_return_val_if_fail (priv->broker != NULL, FALSE);

	if (priv->dict != NULL)
	{
		return TRUE;
	}

	if (priv->active_lang == NULL)
	{
		priv->active_lang = get_default_language ();
	}

	if (priv->active_lang != NULL)
	{
		const gchar *key;

		key = gedit_spell_checker_language_to_key (priv->active_lang);

		priv->dict = enchant_broker_request_dict (priv->broker, key);
	}

	if (priv->dict == NULL)
	{
		priv->active_lang = NULL;
		return FALSE;
	}

	app_name = g_get_application_name ();
	gedit_spell_checker_add_word_to_session (checker, app_name);

	return TRUE;
}

/**
 * gedit_spell_checker_set_language:
 * @checker: a #GeditSpellChecker.
 * @language: (nullable): the #GeditSpellCheckerLanguage to use, or %NULL.
 *
 * Sets the language to use for the spell checking. If @language is %NULL, finds
 * the best available language based on the current locale.
 *
 * Returns: whether the operation was successful.
 */
gboolean
gedit_spell_checker_set_language (GeditSpellChecker               *checker,
				  const GeditSpellCheckerLanguage *language)
{
	GeditSpellCheckerPrivate *priv;
	gboolean success;

	g_return_val_if_fail (GEDIT_IS_SPELL_CHECKER (checker), FALSE);

	priv = gedit_spell_checker_get_instance_private (checker);

	if (language != NULL && priv->active_lang == language)
	{
		return TRUE;
	}

	if (priv->dict != NULL)
	{
		enchant_broker_free_dict (priv->broker, priv->dict);
		priv->dict = NULL;
	}

	priv->active_lang = language;
	success = init_dictionary (checker);

	g_object_notify (G_OBJECT (checker), "language");

	return success;
}

/**
 * gedit_spell_checker_get_language:
 * @checker: a #GeditSpellChecker.
 *
 * Returns: (nullable): the #GeditSpellCheckerLanguage currently used, or %NULL
 * if no dictionaries are available.
 */
const GeditSpellCheckerLanguage *
gedit_spell_checker_get_language (GeditSpellChecker *checker)
{
	GeditSpellCheckerPrivate *priv;

	g_return_val_if_fail (GEDIT_IS_SPELL_CHECKER (checker), NULL);

	priv = gedit_spell_checker_get_instance_private (checker);

	return priv->active_lang;
}

/**
 * gedit_spell_checker_check_word:
 * @checker: a #GeditSpellChecker.
 * @word: the word to check.
 * @error: (out) (optional): a location to a %NULL #GError, or %NULL.
 *
 * Returns: whether @word is correctly spelled.
 */
gboolean
gedit_spell_checker_check_word (GeditSpellChecker  *checker,
				const gchar        *word,
				GError            **error)
{
	GeditSpellCheckerPrivate *priv;
	gint enchant_result;
	gboolean correctly_spelled;

	g_return_val_if_fail (GEDIT_IS_SPELL_CHECKER (checker), FALSE);
	g_return_val_if_fail (word != NULL, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	/* If no dictionaries are available, limit the damage by returning TRUE. */
	g_return_val_if_fail (is_language_set (checker), TRUE);

	priv = gedit_spell_checker_get_instance_private (checker);

	if (gedit_spell_utils_is_digit (word))
	{
		return TRUE;
	}

	enchant_result = enchant_dict_check (priv->dict, word, -1);

	correctly_spelled = enchant_result == 0;

	if (enchant_result < 0)
	{
		g_set_error (error,
			     GEDIT_SPELL_CHECKER_ERROR,
			     GEDIT_SPELL_CHECKER_ERROR_DICTIONARY,
			     _("Error when checking the spelling of word “%s”: %s"),
			     word,
			     enchant_dict_get_error (priv->dict));
	}

	return correctly_spelled;
}

/**
 * gedit_spell_checker_get_suggestions:
 * @checker: a #GeditSpellChecker.
 * @word: a misspelled word.
 *
 * Gets the suggestions for @word. Free the return value with
 * g_slist_free_full(suggestions, g_free).
 *
 * Returns: (transfer full) (element-type utf8): the list of suggestions.
 */
GSList *
gedit_spell_checker_get_suggestions (GeditSpellChecker *checker,
				     const gchar       *word)
{
	GeditSpellCheckerPrivate *priv;
	gchar **suggestions;
	GSList *suggestions_list = NULL;
	gint i;

	g_return_val_if_fail (GEDIT_IS_SPELL_CHECKER (checker), NULL);
	g_return_val_if_fail (word != NULL, NULL);
	g_return_val_if_fail (is_language_set (checker), NULL);

	priv = gedit_spell_checker_get_instance_private (checker);

	suggestions = enchant_dict_suggest (priv->dict, word, -1, NULL);

	if (suggestions == NULL)
	{
		return NULL;
	}

	for (i = 0; suggestions[i] != NULL; i++)
	{
		suggestions_list = g_slist_prepend (suggestions_list, suggestions[i]);
	}

	/* The single suggestions will be freed by the caller */
	g_free (suggestions);

	return g_slist_reverse (suggestions_list);
}

/**
 * gedit_spell_checker_add_word_to_personal:
 * @checker: a #GeditSpellChecker.
 * @word: a word.
 *
 * Adds a word to the personal dictionary. It is typically saved in the user
 * home directory.
 */
void
gedit_spell_checker_add_word_to_personal (GeditSpellChecker *checker,
					  const gchar       *word)
{
	GeditSpellCheckerPrivate *priv;

	g_return_if_fail (GEDIT_IS_SPELL_CHECKER (checker));
	g_return_if_fail (word != NULL);
	g_return_if_fail (is_language_set (checker));

	priv = gedit_spell_checker_get_instance_private (checker);

	enchant_dict_add (priv->dict, word, -1);

	g_signal_emit (G_OBJECT (checker), signals[SIGNAL_ADD_WORD_TO_PERSONAL], 0, word);
}

/**
 * gedit_spell_checker_add_word_to_session:
 * @checker: a #GeditSpellChecker.
 * @word: a word.
 *
 * Adds a word to the session dictionary. The session dictionary is lost when
 * the application exits. This function is typically called when an “Ignore All”
 * action is activated.
 */
void
gedit_spell_checker_add_word_to_session (GeditSpellChecker *checker,
					 const gchar       *word)
{
	GeditSpellCheckerPrivate *priv;

	g_return_if_fail (GEDIT_IS_SPELL_CHECKER (checker));
	g_return_if_fail (word != NULL);
	g_return_if_fail (is_language_set (checker));

	priv = gedit_spell_checker_get_instance_private (checker);

	enchant_dict_add_to_session (priv->dict, word, -1);

	g_signal_emit (G_OBJECT (checker), signals[SIGNAL_ADD_WORD_TO_SESSION], 0, word);
}

/**
 * gedit_spell_checker_clear_session:
 * @checker: a #GeditSpellChecker.
 *
 * Clears the session dictionary.
 */
void
gedit_spell_checker_clear_session (GeditSpellChecker *checker)
{
	GeditSpellCheckerPrivate *priv;

	g_return_if_fail (GEDIT_IS_SPELL_CHECKER (checker));
	g_return_if_fail (is_language_set (checker));

	priv = gedit_spell_checker_get_instance_private (checker);

	/* free and re-request dictionary */
	g_assert (priv->dict != NULL);
	enchant_broker_free_dict (priv->broker, priv->dict);
	priv->dict = NULL;

	init_dictionary (checker);

	g_signal_emit (G_OBJECT (checker), signals[SIGNAL_CLEAR_SESSION], 0);
}

/**
 * gedit_spell_checker_set_correction:
 * @checker: a #GeditSpellChecker.
 * @word: a word.
 * @replacement: the replacement word.
 *
 * Informs the spell checker that @word is replaced/corrected by @replacement.
 */
void
gedit_spell_checker_set_correction (GeditSpellChecker *checker,
				    const gchar       *word,
				    const gchar       *replacement)
{
	GeditSpellCheckerPrivate *priv;

	g_return_if_fail (GEDIT_IS_SPELL_CHECKER (checker));
	g_return_if_fail (word != NULL);
	g_return_if_fail (replacement != NULL);
	g_return_if_fail (is_language_set (checker));

	priv = gedit_spell_checker_get_instance_private (checker);

	enchant_dict_store_replacement (priv->dict,
					word, -1,
					replacement, -1);
}

/* ex:set ts=8 noet: */
