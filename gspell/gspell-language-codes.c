/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2006 - Paolo Maggi
 * Copyright 2008 - Novell, Inc.
 * Copyright 2015, 2016 - Sébastien Wilmet
 * Copyright 2020 - Chun-wei Fan
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n-lib.h>
#include "gspell-utils.h"

#ifdef G_OS_WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

/* Grab the ISO639-2T and ISO3166 codes supported by the system, along with the corresponding language names */
static BOOL CALLBACK
get_win32_all_locales_info (LPWSTR locale_w, DWORD flags, LPARAM param)
{
	gchar *locale = NULL;
	gchar *locale_iso639_2;
	gchar *langname;
	wchar_t *langname_w = NULL;
	wchar_t locale_iso639_2_w[9];
	wchar_t country_code_iso3166_w[9];
	gint langname_size, locale_iso639_2_size, country_code_iso3166_size;
	gboolean success = TRUE;

	GHashTable *ht_iso639_codes = (GHashTable *)param;

	locale = g_utf16_to_utf8 (locale_w, -1,
	                          NULL, NULL, NULL);

	if (locale == NULL)
	{
		success = FALSE;
		goto win32_cb_failed;
	}

	langname_size =
		GetLocaleInfoEx (locale_w,
		                 LOCALE_SLOCALIZEDDISPLAYNAME,
		                 langname_w,
		                 0);

	if (langname_size == 0)
	{
		success = FALSE;
		goto win32_cb_failed;
	}

	langname_w = g_new0 (wchar_t, langname_size);

	if (langname_size == 0)
	{
		success = FALSE;
		goto win32_cb_failed;
	}

	/* First get the translated locale name from the system for the given locale */

	GetLocaleInfoEx (locale_w,
	                 LOCALE_SLOCALIZEDDISPLAYNAME,
	                 langname_w,
	                 langname_size);

	langname = g_utf16_to_utf8 (langname_w, -1,
	                            NULL, NULL, NULL);
	if (langname == NULL)
	{
		success = FALSE;
		goto win32_cb_failed;
	}

	/* now retrieve the 3-letter ISO639-2T language codes */
	locale_iso639_2_size =
		GetLocaleInfoEx (locale_w,
		                 LOCALE_SISO639LANGNAME2,
		                 locale_iso639_2_w,
		                 0);

	if (locale_iso639_2_size > 0)
	{
		GetLocaleInfoEx (locale_w,
		                 LOCALE_SISO639LANGNAME2,
		                 locale_iso639_2_w,
		                 locale_iso639_2_size);

		locale_iso639_2 = g_utf16_to_utf8 (locale_iso639_2_w, -1,
		                                   NULL, NULL, NULL);
	}

	/* and then retrieve the corresponding country codes */
	if (locale_iso639_2 != NULL)
	{
		country_code_iso3166_size =
			GetLocaleInfoEx (locale_w,
			                 LOCALE_SISO3166CTRYNAME2,
			                 country_code_iso3166_w,
			                 0);

		if (country_code_iso3166_size > 0)
		{
			gchar *country_code_iso3166;

			country_code_iso3166_size =
				GetLocaleInfoEx (locale_w,
								LOCALE_SISO3166CTRYNAME2,
								country_code_iso3166_w,
								country_code_iso3166_size);

			country_code_iso3166 = g_utf16_to_utf8 (country_code_iso3166_w, -1,
			                                        NULL, NULL, NULL);

			if (country_code_iso3166 != NULL)
			{
				g_hash_table_insert (ht_iso639_codes,
				                     g_strconcat (locale_iso639_2, "_", country_code_iso3166, NULL),
				                     g_strdup (langname));
			}

			g_free (country_code_iso3166);
		}
		else
		{
			g_hash_table_insert (ht_iso639_codes,
			                     g_strdup (locale_iso639_2),
			                     g_strdup (langname));
		}

		g_free (locale_iso639_2);
	}

win32_cb_failed:
	g_free (langname);
	g_free (langname_w);
	g_free (locale);

	return success;
}

#else

#define ISO_639_DOMAIN	"iso_639"
#define ISO_3166_DOMAIN	"iso_3166"

static gchar *
get_iso_codes_prefix (void)
{
	return g_strdup (ISO_CODES_PREFIX);
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
#endif

static gchar *
_gspell_get_language_name_from_code_in_cb (const gchar * const language_code,
                                           DictsData           *data)
{
	const gchar *iso_639_name;
	const gchar *iso_3166_name;
	gchar *language_name;
	gchar *lowercase;
	gchar **tokens = NULL;

#ifdef G_OS_WIN32
	wchar_t *langname_w = NULL;
	wchar_t *locale_w = NULL;
	int len = 0;
	int i;
	gboolean failed = FALSE;

	locale_w = g_utf8_to_utf16 (language_code, -1, NULL, NULL, NULL);

	if (locale_w != NULL)
		len = GetLocaleInfoEx (locale_w, LOCALE_SLOCALIZEDDISPLAYNAME, langname_w, 0);

	if (len == 0)
	{
		/* Try again, since we may have used an ISO 639-2T language code */
		gchar *name = g_hash_table_lookup (data->iso_639_table, language_code);
		if (name == NULL)
			failed = TRUE;
		else
			language_name = g_strdup (name);

		goto win32_spell_lang_done;
	}

	langname_w = g_new0 (wchar_t, len);

	if (langname_w == NULL ||
		GetLocaleInfoEx (locale_w, LOCALE_SLOCALIZEDDISPLAYNAME, langname_w, len) == 0 ||
        (language_name = g_utf16_to_utf8 (langname_w, -1, NULL, NULL, NULL)) == NULL)
	{
		failed = TRUE;
		goto win32_spell_lang_done;
	}

win32_spell_lang_done:
	if (failed)
	{
		/* Translators: %s is the language ISO code. */
		language_name = g_strdup_printf (C_("language", "Unknown (%s)"), language_code);
	}

	g_free (langname_w);
	g_free (locale_w);

#else
	
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

#endif

	return language_name;
}

void
_gspell_language_dict_describe_cb (const gchar * const language_code,
                                   const gchar * const provider_name,
                                   const gchar * const provider_desc,
                                   const gchar * const provider_file,
                                   DictsData *data)
{
	gchar *language_name = _gspell_get_language_name_from_code_in_cb (language_code, data);

	g_tree_replace (data->tree, g_strdup (language_code), language_name);
}

void
_gspell_populate_language_ht (DictsData *data)
{
	GList *available_languages = NULL;
	gchar *localedir;

	/*
	 * Unfortunately, we still need the ISO639 HashTable because GetLocaleInfoEx()
	 * does not support looking up locale names using ISO 639-2T language codes,
	 * but instead supports using ISO 639-1 codes to look up ISO 639-2T codes,
	 * so we must look up this HashTable if GetLocaleInfoEx() does not find something
	 */
	data->iso_639_table = g_hash_table_new_full (g_str_hash,
						    g_str_equal,
						    (GDestroyNotify) g_free,
						    (GDestroyNotify) g_free);

#ifdef G_OS_WIN32
	/* We could be querying ISO 639-2T locale codes, so grab all the possibilities in the system */
	EnumSystemLocalesEx (&get_win32_all_locales_info,
	                     LOCALE_ALL,
	                     (LPARAM)data->iso_639_table,
	                     NULL);
#else
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

	localedir = get_iso_codes_localedir ();

	bindtextdomain (ISO_639_DOMAIN, localedir);
	bind_textdomain_codeset (ISO_639_DOMAIN, "UTF-8");

	bindtextdomain (ISO_3166_DOMAIN, localedir);
	bind_textdomain_codeset (ISO_3166_DOMAIN, "UTF-8");

	g_free (localedir);

	data->iso_3166_table = g_hash_table_new_full (g_str_hash,
						     g_str_equal,
						     (GDestroyNotify) g_free,
						     (GDestroyNotify) g_free);

	iso_codes_parse (&iso_639_parser, "iso_639.xml", data->iso_639_table);
	iso_codes_parse (&iso_3166_parser, "iso_3166.xml", data->iso_3166_table);
#endif
}
