/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2006 - Paolo Maggi
 * Copyright 2008 - Novell, Inc.
 * Copyright 2015, 2016 - Sébastien Wilmet
 * Copyright 2018 - Chun-wei Fan
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
#include "gspell-locales.h"
#include "gspell-init.h"

#define ISO_639_DOMAIN	"iso_639"
#define ISO_3166_DOMAIN	"iso_3166"

struct _GspellLocales
{
	GObject parent;

	GHashTable *iso_639_table;
	GHashTable *iso_3166_table;
	guint iso_639_inited : 1;
	guint iso_3166_inited : 1;
};

enum _GspellLocaleInfoType
{
	GSPELL_ISO_639,
	GSPELL_ISO_3166
};

typedef enum _GspellLocaleInfoType GspellLocaleInfoType;

G_DEFINE_TYPE (GspellLocales, gspell_locales, G_TYPE_OBJECT)

static void
gspell_locales_init (GspellLocales *gspell_locales)
{
}

static void
gspell_locales_dispose (GObject *object)
{
	G_OBJECT_CLASS (gspell_locales_parent_class)->dispose (object);
}

static void
gspell_locales_finalize (GObject *object)
{
	GspellLocales *locales = GSPELL_LOCALES (object);

	g_hash_table_unref (locales->iso_639_table);
	g_hash_table_unref (locales->iso_3166_table);

	G_OBJECT_CLASS (gspell_locales_parent_class)->finalize (object);
}

static void
gspell_locales_class_init (GspellLocalesClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = gspell_locales_dispose;
	object_class->finalize = gspell_locales_finalize;
}

#ifdef G_OS_WIN32
typedef struct _LocaleData
{
	GspellLocales *locales;
	GspellLocaleInfoType type;
} LocaleData;

/* if we are using native Windows, use Windows API
 * to retrieve the translated language and country names
 * (No need for iso-codes package)
 */
static BOOL CALLBACK
get_win32_all_locales_info (LPWSTR locale_w, DWORD flags, LPARAM param)
{
	wchar_t *langname_w = NULL;
	wchar_t *locale_abbrev_w = NULL;
	gchar *langname, *locale_abbrev, *locale, *countrycode;
	gint i;
	LocaleData *data = (LocaleData *)param;
	GspellLocales *locales = data->locales;

	gint langname_size, locale_abbrev_size;
	locale = g_utf16_to_utf8 (locale_w, -1,
	                          NULL, NULL, NULL);

	if (data->type == GSPELL_ISO_639)
	{
		langname_size =
			GetLocaleInfoEx (locale_w,
			                 LOCALE_SLOCALIZEDDISPLAYNAME,
			                 langname_w,
			                 0);

		if (langname_size == 0)
			return FALSE;

		langname_w = g_new0 (wchar_t, langname_size);

		if (langname_size == 0)
			return FALSE;

		GetLocaleInfoEx (locale_w,
		                 LOCALE_SLOCALIZEDDISPLAYNAME,
		                 langname_w,
		                 langname_size);

		langname = g_utf16_to_utf8 (langname_w, -1,
		                            NULL, NULL, NULL);

		g_hash_table_insert (locales->iso_639_table,
		                     g_strdup (locale),
		                     g_strdup (langname));

		/* track 3-letter ISO639-2/3 language codes as well */
		locale_abbrev_size =
			GetLocaleInfoEx (locale_w,
			                 LOCALE_SABBREVLANGNAME,
			                 locale_abbrev_w,
			                 0);

		if (locale_abbrev_size > 0)
		{
			locale_abbrev_w = g_new0 (wchar_t, locale_abbrev_size);
			GetLocaleInfoEx (locale_w,
			                 LOCALE_SABBREVLANGNAME,
			                 locale_abbrev_w,
			                 locale_abbrev_size);

			locale_abbrev = g_utf16_to_utf8 (locale_abbrev_w, -1,
			                                 NULL, NULL, NULL);

			g_hash_table_insert (locales->iso_639_table,
			                     g_strdup (locale_abbrev),
			                     g_strdup (langname));

			g_free (locale_abbrev);
			g_free (locale_abbrev_w);
		}
	}
	else if (data->type == GSPELL_ISO_3166)
	{
		countrycode = strrchr (locale, '-') != NULL ?
		              strrchr (locale, '-') + sizeof (gchar) :
		              NULL;

		if (countrycode != NULL)
		{
			wchar_t *countryname_w = NULL;
			gchar *countryname = NULL;
			gint countryname_size;

			countryname_size =
				GetLocaleInfoEx (locale_w,
				                 LOCALE_SLOCALIZEDCOUNTRYNAME,
				                 countryname_w,
				                 0);

			if (countryname_size > 0)
			{
				countryname_w = g_new0 (wchar_t, countryname_size);
				countryname_size =
					GetLocaleInfoEx (locale_w,
					                 LOCALE_SLOCALIZEDCOUNTRYNAME,
					                 countryname_w,
					                 countryname_size);

				countryname = g_utf16_to_utf8 (countryname_w, -1,
					                           NULL, NULL, NULL);

				g_hash_table_insert (locales->iso_3166_table,
				                     g_ascii_strdown (countrycode, -1),
				                     g_strdup (countryname));

				g_free (countryname);
				g_free (countryname_w);
			}
		}
	}

	g_free (langname);
	g_free (locale);
	g_free (langname_w);

	return TRUE;
}

static void
gspell_retrieve_locale_info (GspellLocales *locales,
                             GspellLocaleInfoType type)
{
	LocaleData data;
	g_return_if_fail (type == GSPELL_ISO_639 ||
	                  type == GSPELL_ISO_3166);

	data.locales = locales;
	data.type = type;

	EnumSystemLocalesEx (&get_win32_all_locales_info,
	                     LOCALE_ALL,
	                     (LPARAM)&data,
	                     NULL);

	switch (type)
	{
		case GSPELL_ISO_639:
			locales->iso_639_inited = TRUE;
			break;
		case GSPELL_ISO_3166:
			locales->iso_3166_inited = TRUE;
			break;
		default:
			g_warning ("Should not have reached here!");
	}
}

#else /* G_OS_WIN32 */
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

static void
gspell_retrieve_locale_info (GspellLocales *locales,
                             GspellLocaleInfoType type)
{
	gchar *localedir;
	GMarkupParser iso_markup_parser = {NULL};
	gchar *i18n_domain = NULL;
	gchar *i18n_domain_xml = NULL;
	GHashTable *saved_info_table = NULL;

	g_return_if_fail (type == GSPELL_ISO_639 || type == GSPELL_ISO_3166);

	localedir = get_iso_codes_localedir ();

	switch (type)
	{
		case GSPELL_ISO_639:
			iso_markup_parser.start_element = iso_639_start_element;
			i18n_domain = g_strdup (ISO_639_DOMAIN);
			i18n_domain_xml = g_strconcat (ISO_639_DOMAIN, ".xml", NULL);
			saved_info_table = locales->iso_639_table;
			locales->iso_639_inited = TRUE;
			break;

		case GSPELL_ISO_3166:
			iso_markup_parser.start_element = iso_3166_start_element;
			i18n_domain = g_strdup (ISO_3166_DOMAIN);
			i18n_domain_xml = g_strconcat (ISO_3166_DOMAIN, ".xml", NULL);
			saved_info_table = locales->iso_3166_table;
			locales->iso_3166_inited = TRUE;
			break;

		default:
			g_warning ("Should not get here!\n");
	}

	bindtextdomain (i18n_domain, localedir);
	bind_textdomain_codeset (i18n_domain, "UTF-8");

	g_free (localedir);

	iso_codes_parse (&iso_markup_parser,
	                 i18n_domain_xml,
	                 saved_info_table);

	g_free (i18n_domain_xml);
	g_free (i18n_domain);
}

#endif /* ! G_OS_WIN32 */

GHashTable *
gspell_locales_get_iso_639_names (GspellLocales *locales)
{
	if (locales->iso_639_inited)
		return locales->iso_639_table;

	locales->iso_639_table = g_hash_table_new_full (g_str_hash,
						     g_str_equal,
						     (GDestroyNotify) g_free,
						     (GDestroyNotify) g_free);

	gspell_retrieve_locale_info (locales, GSPELL_ISO_639);

	return locales->iso_639_table;
}

GHashTable *
gspell_locales_get_iso_3166_names (GspellLocales *locales)
{
	if (locales->iso_3166_inited)
		return locales->iso_3166_table;

	locales->iso_3166_table = g_hash_table_new_full (g_str_hash,
						     g_str_equal,
						     (GDestroyNotify) g_free,
						     (GDestroyNotify) g_free);

	gspell_retrieve_locale_info (locales, GSPELL_ISO_3166);

	return locales->iso_3166_table;
}
