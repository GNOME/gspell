/* SPDX-FileCopyrightText: 2015, 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell-config.h"
#include "gspell-language-chooser.h"

/**
 * SECTION:language-chooser
 * @Short_description: Interface to choose a GspellLanguage
 * @Title: GspellLanguageChooser
 * @See_also: #GspellLanguage, #GspellLanguageChooserButton, #GspellLanguageChooserDialog
 *
 * #GspellLanguageChooser is an interface that is implemented by widgets for
 * choosing a #GspellLanguage.
 *
 * There are two properties: #GspellLanguageChooser:language and
 * #GspellLanguageChooser:language-code. They are kept in sync. The former is
 * useful, for example, to bind it to the #GspellChecker's language property
 * with g_object_bind_property(). The latter is useful to bind it to a
 * #GSettings key with g_settings_bind().
 *
 * When setting the language, %NULL or the empty string can be passed to pick
 * the default language. In that case, the #GspellLanguageChooser:language-code
 * property will contain the empty string, whereas the
 * #GspellLanguageChooser:language property will contain the actual
 * #GspellLanguage as returned by gspell_language_get_default(). If the user
 * launches the #GspellLanguageChooser and chooses explicitly a language, then
 * the #GspellLanguageChooser:language-code property will no longer be empty,
 * even if it is the same language as the default language.
 *
 * Note that if an explicit language (non-%NULL or not the empty string) is set
 * to the #GspellLanguageChooser, then the #GspellLanguageChooser:language-code
 * property will not be empty, it will contain the language code of the passed
 * language, even if the language is the same as the default language.
 *
 * Thus, a good default value for a #GSettings key is the empty string. That
 * way, the default language is picked, and can change depending on the locale.
 * But once the user has chosen a language, that language is kept in the
 * #GSettings key.
 */

G_DEFINE_INTERFACE (GspellLanguageChooser, gspell_language_chooser, G_TYPE_OBJECT)

static void
gspell_language_chooser_default_init (GspellLanguageChooserInterface *interface)
{
	/**
	 * GspellLanguageChooser:language:
	 *
	 * The selected #GspellLanguage.
	 */
	g_object_interface_install_property (interface,
					     g_param_spec_boxed ("language",
								 "Language",
								 "",
								 GSPELL_TYPE_LANGUAGE,
								 G_PARAM_READWRITE |
								 G_PARAM_STATIC_STRINGS));

	/**
	 * GspellLanguageChooser:language-code:
	 *
	 * The empty string if the default language was set and the selection
	 * hasn't changed. Or the language code if an explicit language was set
	 * or if the selection has changed.
	 */
	g_object_interface_install_property (interface,
					     g_param_spec_string ("language-code",
								  "Language Code",
								  "",
								  "",
								  G_PARAM_READWRITE |
								  G_PARAM_STATIC_STRINGS));
}

/**
 * gspell_language_chooser_get_language:
 * @chooser: a #GspellLanguageChooser.
 *
 * Returns: (nullable): the selected #GspellLanguage, or %NULL if no
 * dictionaries are available.
 */
const GspellLanguage *
gspell_language_chooser_get_language (GspellLanguageChooser *chooser)
{
	g_return_val_if_fail (GSPELL_IS_LANGUAGE_CHOOSER (chooser), NULL);

	return GSPELL_LANGUAGE_CHOOSER_GET_IFACE (chooser)->get_language_full (chooser, NULL);
}

/**
 * gspell_language_chooser_set_language:
 * @chooser: a #GspellLanguageChooser.
 * @language: (nullable): a #GspellLanguage or %NULL to pick the default
 *   language.
 *
 * Sets the selected language.
 */
void
gspell_language_chooser_set_language (GspellLanguageChooser *chooser,
				      const GspellLanguage  *language)
{
	g_return_if_fail (GSPELL_IS_LANGUAGE_CHOOSER (chooser));

	GSPELL_LANGUAGE_CHOOSER_GET_IFACE (chooser)->set_language (chooser, language);
}

/**
 * gspell_language_chooser_get_language_code:
 * @chooser: a #GspellLanguageChooser.
 *
 * Returns: the #GspellLanguageChooser:language-code. It cannot be %NULL.
 */
const gchar *
gspell_language_chooser_get_language_code (GspellLanguageChooser *chooser)
{
	const GspellLanguage *lang;
	gboolean default_lang = TRUE;
	const gchar *language_code;

	g_return_val_if_fail (GSPELL_IS_LANGUAGE_CHOOSER (chooser), "");

	lang = GSPELL_LANGUAGE_CHOOSER_GET_IFACE (chooser)->get_language_full (chooser, &default_lang);

	if (default_lang || lang == NULL)
	{
		return "";
	}

	language_code = gspell_language_get_code (lang);
	g_return_val_if_fail (language_code != NULL, "");

	return language_code;
}

/**
 * gspell_language_chooser_set_language_code:
 * @chooser: a #GspellLanguageChooser.
 * @language_code: (nullable): a language code, or the empty string or %NULL to
 *   pick the default language.
 */
void
gspell_language_chooser_set_language_code (GspellLanguageChooser *chooser,
					   const gchar           *language_code)
{
	const GspellLanguage *lang = NULL;

	g_return_if_fail (GSPELL_IS_LANGUAGE_CHOOSER (chooser));

	if (language_code != NULL && language_code[0] != '\0')
	{
		lang = gspell_language_lookup (language_code);
	}

	GSPELL_LANGUAGE_CHOOSER_GET_IFACE (chooser)->set_language (chooser, lang);
}
