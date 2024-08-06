/* SPDX-FileCopyrightText: 2006 - Paolo Maggi
 * SPDX-FileCopyrightText: 2008 - Novell, Inc.
 * SPDX-FileCopyrightText: 2015, 2016, 2020 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell-config.h"
#include "gspell-language.h"
#include <string.h>
#include <glib/gi18n-lib.h>
#include <enchant.h>
#include "gspell-icu.h"

#if OS_MACOS
#include "gspell-osx.h"
#endif

/**
 * SECTION:language
 * @Short_description: Language
 * @Title: GspellLanguage
 * @See_also: #GspellChecker
 *
 * #GspellLanguage represents a language that can be used for the spell
 * checking, i.e. a language for which a dictionary is installed.
 */

struct _GspellLanguage
{
	gchar *code;
	gchar *name;
	gchar *collate_key;
};

G_DEFINE_BOXED_TYPE (GspellLanguage,
		     gspell_language,
		     gspell_language_copy,
		     gspell_language_free)

static void
spell_language_dict_describe_cb (const gchar * const language_code,
                                 const gchar * const provider_name,
                                 const gchar * const provider_desc,
                                 const gchar * const provider_file,
				 gpointer            user_data)
{
	GList **available_languages = user_data;
	GList *l;
	GspellLanguage *language;

	g_return_if_fail (language_code != NULL);

	for (l = *available_languages; l != NULL; l = l->next)
	{
		GspellLanguage *cur_language = l->data;

		if (g_strcmp0 (cur_language->code, language_code) == 0)
		{
			/* Avoid duplicates. */
			return;
		}
	}

	language = g_new0 (GspellLanguage, 1);
	language->code = g_strdup (language_code);

	language->name = _gspell_icu_get_language_name_from_code (language_code, NULL);
	if (language->name == NULL)
	{
		/* Translators: %s is the language ISO code. */
		language->name = g_strdup_printf (C_("language", "Unknown (%s)"), language_code);
	}

	language->collate_key = g_utf8_collate_key (language->name, -1);

	*available_languages = g_list_prepend (*available_languages, language);
}

/**
 * gspell_language_get_available:
 *
 * Returns: (transfer none) (element-type GspellLanguage): the list of available
 * languages, sorted with gspell_language_compare().
 */
const GList *
gspell_language_get_available (void)
{
	static gboolean initialized = FALSE;
	static GList *available_languages = NULL;
	EnchantBroker *broker;

	if (initialized)
	{
		return available_languages;
	}

	initialized = TRUE;

	broker = enchant_broker_init ();
	enchant_broker_list_dicts (broker,
				   spell_language_dict_describe_cb,
				   &available_languages);
	enchant_broker_free (broker);

	available_languages = g_list_sort (available_languages,
					   (GCompareFunc) gspell_language_compare);

	return available_languages;
}

/**
 * gspell_language_get_default:
 *
 * Finds the best available language based on the current locale.
 *
 * Returns: (nullable): the default #GspellLanguage, or %NULL if no dictionaries
 * are available.
 */
const GspellLanguage *
gspell_language_get_default (void)
{
	const GspellLanguage *lang;
	const gchar * const *lang_names;
	const GList *langs;
	gint i;

	/* Try with the current locale */
	lang_names = g_get_language_names ();

	for (i = 0; lang_names[i] != NULL; i++)
	{
		lang = gspell_language_lookup (lang_names[i]);

		if (lang != NULL)
		{
			return lang;
		}
	}

	/* Another try specific to Mac OS X */
#if OS_MACOS
	{
		gchar *code = _gspell_osx_get_preferred_spell_language ();

		if (code != NULL)
		{
			lang = gspell_language_lookup (code);
			g_free (code);

			if (lang != NULL)
			{
				return lang;
			}
		}
	}
#endif

	/* Try English */
	lang = gspell_language_lookup ("en_US");
	if (lang != NULL)
	{
		return lang;
	}

	/* Take the first available language */
	langs = gspell_language_get_available ();
	if (langs != NULL)
	{
		return langs->data;
	}

	return NULL;
}

/**
 * gspell_language_lookup:
 * @language_code: a language code.
 *
 * Returns: (nullable): a #GspellLanguage corresponding to @language_code, or
 * %NULL if not found.
 */
const GspellLanguage *
gspell_language_lookup (const gchar *language_code)
{
	const GspellLanguage *closest_match = NULL;
	const GList *available_languages;
	const GList *l;

	g_return_val_if_fail (language_code != NULL, NULL);

	available_languages = gspell_language_get_available ();

	for (l = available_languages; l != NULL; l = l->next)
	{
		const GspellLanguage *language = l->data;
		const gchar *code = language->code;
		gsize length = strlen (code);

		if (g_ascii_strcasecmp (language_code, code) == 0)
		{
			return language;
		}

		if (g_ascii_strncasecmp (language_code, code, length) == 0)
		{
			closest_match = language;
		}
	}

	return closest_match;
}

/**
 * gspell_language_get_code:
 * @language: a #GspellLanguage.
 *
 * Returns: the @language code, for example fr_BE.
 */
const gchar *
gspell_language_get_code (const GspellLanguage *language)
{
	g_return_val_if_fail (language != NULL, NULL);

	return language->code;
}

/**
 * gspell_language_get_name:
 * @language: a #GspellLanguage.
 *
 * Returns the @language name translated to the current locale. For example
 * "French (Belgium)" is returned if the current locale is in English and the
 * @language code is fr_BE.
 *
 * Returns: the @language name.
 */
const gchar *
gspell_language_get_name (const GspellLanguage *language)
{
	g_return_val_if_fail (language != NULL, NULL);

	return language->name;
}

/**
 * gspell_language_compare:
 * @language_a: a #GspellLanguage.
 * @language_b: another #GspellLanguage.
 *
 * Compares alphabetically two languages by their name, as returned by
 * gspell_language_get_name().
 *
 * Returns: an integer less than, equal to, or greater than zero, if @language_a
 * is <, == or > than @language_b.
 */
gint
gspell_language_compare (const GspellLanguage *language_a,
                         const GspellLanguage *language_b)
{
	g_return_val_if_fail (language_a != NULL, 0);
	g_return_val_if_fail (language_b != NULL, 0);

	return g_strcmp0 (language_a->collate_key, language_b->collate_key);
}

/**
 * gspell_language_copy:
 * @language: a #GspellLanguage.
 *
 * Used by language bindings.
 *
 * Returns: a copy of @lang.
 */
GspellLanguage *
gspell_language_copy (const GspellLanguage *language)
{
	g_return_val_if_fail (language != NULL, NULL);

	return (GspellLanguage *) language;
}

/**
 * gspell_language_free:
 * @language: a #GspellLanguage.
 *
 * Used by language bindings.
 */
void
gspell_language_free (GspellLanguage *language)
{
	g_return_if_fail (language != NULL);
}
