/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2006 - Paolo Maggi
 * Copyright 2008 - Novell, Inc.
 * Copyright 2015, 2016 - Sébastien Wilmet
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "gspell-language.h"
#include <string.h>
#include <glib/gi18n-lib.h>
#include <enchant.h>
#include "gspell-init.h"

#ifdef OS_OSX
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

#define ISO_639_DOMAIN	"iso_639"
#define ISO_3166_DOMAIN	"iso_3166"

struct _GspellLanguage
{
	gchar *code;
	gchar *name;
	gchar *collate_key;
};

typedef struct _DictsData DictsData;
struct _DictsData
{
	GHashTable *iso_639_table;
	GHashTable *iso_3166_table;
	GTree *tree;
};

G_DEFINE_BOXED_TYPE (GspellLanguage,
		     gspell_language,
		     gspell_language_copy,
		     gspell_language_free)

static gchar *
get_iso_codes_prefix (void)
{
	gchar *prefix = NULL;

#ifdef G_OS_WIN32
	HMODULE gspell_dll;

	gspell_dll = _gspell_init_get_dll ();
	prefix = g_win32_get_package_installation_directory_of_module ((gpointer) gspell_dll);
#endif

	if (prefix == NULL)
	{
		prefix = g_strdup (ISO_CODES_PREFIX);
	}

	return prefix;
}

static gchar *
get_iso_codes_localedir (void)
{
	gchar *prefix;
	gchar *localedir;

	prefix = get_iso_codes_prefix ();
	localedir = g_build_filename (prefix, "share", "locale", NULL);
	g_free (prefix);

	return localedir;
}

static void
iso_639_start_element (GMarkupParseContext  *context,
		       const gchar          *element_name,
		       const gchar         **attribute_names,
		       const gchar         **attribute_values,
		       gpointer              data,
		       GError              **error)
{
	GHashTable *hash_table = data;
	const gchar *iso_639_1_code = NULL;
	const gchar *iso_639_2_code = NULL;
	const gchar *name = NULL;
	const gchar *code = NULL;
	gint i;

	if (g_strcmp0 (element_name, "iso_639_entry") != 0)
	{
		return;
	}

	for (i = 0; attribute_names[i] != NULL; i++)
	{
		if (g_str_equal (attribute_names[i], "name"))
		{
			name = attribute_values[i];
		}
		else if (g_str_equal (attribute_names[i], "iso_639_1_code"))
		{
			iso_639_1_code = attribute_values[i];
		}
		else if (g_str_equal (attribute_names[i], "iso_639_2T_code"))
		{
			iso_639_2_code = attribute_values[i];
		}
	}

	code = (iso_639_1_code != NULL) ? iso_639_1_code : iso_639_2_code;

	if (code != NULL && code[0] != '\0' &&
	    name != NULL && name[0] != '\0')
	{
		g_hash_table_insert (hash_table,
				     g_strdup (code),
				     g_strdup (dgettext (ISO_639_DOMAIN, name)));
	}
}

static void
iso_3166_start_element (GMarkupParseContext  *context,
			const gchar          *element_name,
			const gchar         **attribute_names,
			const gchar         **attribute_values,
			gpointer              data,
			GError              **error)
{
	GHashTable *hash_table = data;
	const gchar *name = NULL;
	const gchar *code = NULL;
	gint i;

	if (g_strcmp0 (element_name, "iso_3166_entry") != 0)
	{
		return;
	}

	for (i = 0; attribute_names[i] != NULL; i++)
	{
		if (g_str_equal (attribute_names[i], "name"))
		{
			name = attribute_values[i];
		}
		else if (g_str_equal (attribute_names[i], "alpha_2_code"))
		{
			code = attribute_values[i];
		}
	}

	if (code != NULL && code[0] != '\0' &&
	    name != NULL && name[0] != '\0')
	{
		g_hash_table_insert (hash_table,
				     g_ascii_strdown (code, -1),
				     g_strdup (dgettext (ISO_3166_DOMAIN, name)));
	}
}

static void
iso_codes_parse (const GMarkupParser *parser,
		 const gchar         *basename,
		 GHashTable          *hash_table)
{
	GMappedFile *mapped_file;
	gchar *prefix;
	gchar *filename;
	GError *error = NULL;

	prefix = get_iso_codes_prefix ();
	filename = g_build_filename (prefix,
				     "share",
				     "xml",
				     "iso-codes",
				     basename,
				     NULL);
	g_free (prefix);

	mapped_file = g_mapped_file_new (filename, FALSE, &error);
	g_free (filename);

	if (mapped_file != NULL)
	{
		GMarkupParseContext *context;
		const gchar *contents;
		gsize length;

		context = g_markup_parse_context_new (parser, 0, hash_table, NULL);
		contents = g_mapped_file_get_contents (mapped_file);
		length = g_mapped_file_get_length (mapped_file);
		g_markup_parse_context_parse (context, contents, length, &error);
		g_markup_parse_context_free (context);
		g_mapped_file_unref (mapped_file);
	}

	if (error != NULL)
	{
		g_warning ("%s: %s", basename, error->message);
		g_clear_error (&error);
	}
}

static void
spell_language_dict_describe_cb (const gchar * const language_code,
                                 const gchar * const provider_name,
                                 const gchar * const provider_desc,
                                 const gchar * const provider_file,
				 DictsData *data)
{
	const gchar *iso_639_name;
	const gchar *iso_3166_name;
	gchar *language_name;
	gchar *lowercase;
	gchar **tokens;

	/* Split language code into lowercase tokens. */
	lowercase = g_ascii_strdown (language_code, -1);
	tokens = g_strsplit (lowercase, "_", -1);
	g_free (lowercase);

	g_return_if_fail (tokens != NULL);

	iso_639_name = g_hash_table_lookup (data->iso_639_table, tokens[0]);

	if (iso_639_name == NULL)
	{
		/* Translators: %s is the language ISO code. */
		language_name = g_strdup_printf (C_("language", "Unknown (%s)"), language_code);
		goto exit;
	}

	if (g_strv_length (tokens) < 2)
	{
		language_name = g_strdup (iso_639_name);
		goto exit;
	}

	iso_3166_name = g_hash_table_lookup (data->iso_3166_table, tokens[1]);

	if (iso_3166_name != NULL)
	{
		/* Translators: The first %s is the language name, and the
		 * second is the country name. Example: "French (France)".
		 */
		language_name = g_strdup_printf (C_("language", "%s (%s)"),
						 iso_639_name,
						 iso_3166_name);
	}
	else
	{
		/* Translators: The first %s is the language name, and the
		 * second is the country name. Example: "French (France)".
		 */
		language_name = g_strdup_printf (C_("language", "%s (%s)"),
						 iso_639_name,
						 tokens[1]);
	}

exit:
	g_strfreev (tokens);

	g_tree_replace (data->tree, g_strdup (language_code), language_name);
}

static gboolean
spell_language_traverse_cb (const gchar  *code,
			    const gchar  *name,
			    GList       **available_languages)
{
	GspellLanguage *language;

	language = g_slice_new (GspellLanguage);
	language->code = g_strdup (code);
	language->name = g_strdup (name);
	language->collate_key = g_utf8_collate_key (name, -1);

	*available_languages = g_list_insert_sorted (*available_languages,
						     language,
						     (GCompareFunc) gspell_language_compare);

	return FALSE;
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
	gchar *localedir;
	EnchantBroker *broker;
	DictsData data;

	GMarkupParser iso_639_parser =
	{
		iso_639_start_element,
		NULL, NULL, NULL, NULL
	};

	GMarkupParser iso_3166_parser =
	{
		iso_3166_start_element,
		NULL, NULL, NULL, NULL
	};

	if (initialized)
	{
		return available_languages;
	}

	initialized = TRUE;

	localedir = get_iso_codes_localedir ();

	bindtextdomain (ISO_639_DOMAIN, localedir);
	bind_textdomain_codeset (ISO_639_DOMAIN, "UTF-8");

	bindtextdomain (ISO_3166_DOMAIN, localedir);
	bind_textdomain_codeset (ISO_3166_DOMAIN, "UTF-8");

	g_free (localedir);

	data.iso_639_table = g_hash_table_new_full (g_str_hash,
						    g_str_equal,
						    (GDestroyNotify) g_free,
						    (GDestroyNotify) g_free);

	data.iso_3166_table = g_hash_table_new_full (g_str_hash,
						     g_str_equal,
						     (GDestroyNotify) g_free,
						     (GDestroyNotify) g_free);

	iso_codes_parse (&iso_639_parser, "iso_639.xml", data.iso_639_table);
	iso_codes_parse (&iso_3166_parser, "iso_3166.xml", data.iso_3166_table);

	data.tree = g_tree_new_full ((GCompareDataFunc) strcmp,
				     NULL,
				     (GDestroyNotify) g_free,
				     (GDestroyNotify) g_free);

	broker = enchant_broker_init ();
	enchant_broker_list_dicts (broker,
				   (EnchantDictDescribeFn) spell_language_dict_describe_cb,
				   &data);
	enchant_broker_free (broker);

	g_tree_foreach (data.tree,
			(GTraverseFunc) spell_language_traverse_cb,
			&available_languages);

	g_hash_table_unref (data.iso_639_table);
	g_hash_table_unref (data.iso_3166_table);
	g_tree_unref (data.tree);

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
#ifdef OS_OSX
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

/* ex:set ts=8 noet: */
