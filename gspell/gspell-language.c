/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2006 - Paolo Maggi
 * Copyright 2008 - Novell, Inc.
 * Copyright 2015 - Sébastien Wilmet
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

#define ISO_639_DOMAIN	"iso_639"
#define ISO_3166_DOMAIN	"iso_3166"

struct _GspellLanguage
{
	gchar *code;
	gchar *name;
	gchar *ckey;
};

G_DEFINE_BOXED_TYPE (GspellLanguage,
		     gspell_language,
		     gspell_language_copy,
		     gspell_language_free)

static GHashTable *iso_639_table = NULL;
static GHashTable *iso_3166_table = NULL;

#ifdef G_OS_WIN32

#ifdef DATADIR
#undef DATADIR
#endif

#include <shlobj.h>
static HMODULE hmodule;

BOOL WINAPI
DllMain (HINSTANCE hinstDLL,
         DWORD     fdwReason,
         LPVOID    lpvReserved);

BOOL WINAPI
DllMain (HINSTANCE hinstDLL,
	 DWORD     fdwReason,
	 LPVOID    lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			hmodule = hinstDLL;
			break;
	}

	return TRUE;
}

#endif /* G_OS_WIN32 */

static gchar *
get_iso_codes_prefix (void)
{
	gchar *prefix = NULL;

#ifdef G_OS_WIN32
	prefix = g_win32_get_package_installation_directory_of_module ((gpointer) hmodule);
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
	gint ii;

	if (strcmp (element_name, "iso_639_entry") != 0)
	{
		return;
	}

	for (ii = 0; attribute_names[ii] != NULL; ii++)
	{
		if (strcmp (attribute_names[ii], "name") == 0)
		{
			name = attribute_values[ii];
		}
		else if (strcmp (attribute_names[ii], "iso_639_1_code") == 0)
		{
			iso_639_1_code = attribute_values[ii];
		}
		else if (strcmp (attribute_names[ii], "iso_639_2T_code") == 0)
		{
			iso_639_2_code = attribute_values[ii];
		}
	}

	code = (iso_639_1_code != NULL) ? iso_639_1_code : iso_639_2_code;

	if (code != NULL && *code != '\0' &&
	    name != NULL && *name != '\0')
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
	gint ii;

	if (strcmp (element_name, "iso_3166_entry") != 0)
	{
		return;
	}

	for (ii = 0; attribute_names[ii] != NULL; ii++)
	{
		if (strcmp (attribute_names[ii], "name") == 0)
		{
			name = attribute_values[ii];
		}
		else if (strcmp (attribute_names[ii], "alpha_2_code") == 0)
		{
			code = attribute_values[ii];
		}
	}

	if (code != NULL && *code != '\0' &&
	    name != NULL && *name != '\0')
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
		g_error_free (error);
	}
}

static void
spell_language_dict_describe_cb (const gchar * const language_code,
                                 const gchar * const provider_name,
                                 const gchar * const provider_desc,
                                 const gchar * const provider_file,
                                 GTree *tree)
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

	iso_639_name = g_hash_table_lookup (iso_639_table, tokens[0]);

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

	iso_3166_name = g_hash_table_lookup (iso_3166_table, tokens[1]);

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

	g_tree_replace (tree, g_strdup (language_code), language_name);
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
	language->ckey = g_utf8_collate_key (name, -1);

	*available_languages = g_list_insert_sorted (*available_languages,
						     language,
						     (GCompareFunc) gspell_language_compare);

	return FALSE;
}

/**
 * gspell_language_get_available:
 *
 * Returns: (transfer none) (element-type GspellLanguage): the list of available
 * languages for the spell checking.
 */
const GList *
gspell_language_get_available (void)
{
	static gboolean initialized = FALSE;
	static GList *available_languages = NULL;
	gchar *localedir;
	EnchantBroker *broker;
	GTree *tree;

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

	iso_639_table = g_hash_table_new_full (g_str_hash,
					       g_str_equal,
					       (GDestroyNotify) g_free,
					       (GDestroyNotify) g_free);

	iso_3166_table = g_hash_table_new_full (g_str_hash,
						g_str_equal,
						(GDestroyNotify) g_free,
						(GDestroyNotify) g_free);

	iso_codes_parse (&iso_639_parser, "iso_639.xml", iso_639_table);
	iso_codes_parse (&iso_3166_parser, "iso_3166.xml", iso_3166_table);

	tree = g_tree_new_full ((GCompareDataFunc) strcmp,
				NULL,
				(GDestroyNotify) g_free,
				(GDestroyNotify) g_free);

	broker = enchant_broker_init ();
	enchant_broker_list_dicts (broker,
				   (EnchantDictDescribeFn) spell_language_dict_describe_cb,
				   tree);
	enchant_broker_free (broker);

	g_tree_foreach (tree,
			(GTraverseFunc) spell_language_traverse_cb,
			&available_languages);

	g_tree_destroy (tree);

	return available_languages;
}

const GspellLanguage *
gspell_language_lookup (const gchar *language_code)
{
	const GspellLanguage *closest_match = NULL;
	const GList *available_languages;

	g_return_val_if_fail (language_code != NULL, NULL);

	available_languages = gspell_language_get_available ();

	while (available_languages != NULL)
	{
		GspellLanguage *language = available_languages->data;
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

		available_languages = g_list_next (available_languages);
	}

	return closest_match;
}

const gchar *
gspell_language_get_code (const GspellLanguage *language)
{
	g_return_val_if_fail (language != NULL, NULL);

	return language->code;
}

const gchar *
gspell_language_get_name (const GspellLanguage *language)
{
	if (language == NULL)
	{
		/* Translators: This refers to the default language used by the
		 * spell checker.
		 */
		return C_("language", "Default");
	}

	return language->name;
}

gint
gspell_language_compare (const GspellLanguage *language_a,
                         const GspellLanguage *language_b)
{
	g_return_val_if_fail (language_a != NULL, 0);
	g_return_val_if_fail (language_b != NULL, 0);

	return g_strcmp0 (language_a->ckey, language_b->ckey);
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
