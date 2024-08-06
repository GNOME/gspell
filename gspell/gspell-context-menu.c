/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell-config.h"
#include "gspell-context-menu.h"
#include <glib/gi18n-lib.h>

#define LANGUAGE_DATA_KEY "gspell-language-data-key"
#define SUGGESTION_DATA_KEY "gspell-suggestion-data-key"

typedef struct _LanguageData	LanguageData;
typedef struct _SuggestionData	SuggestionData;

struct _LanguageData
{
	const GspellLanguage *lang;
	GspellLanguageActivatedCallback callback;
	gpointer user_data;
};

struct _SuggestionData
{
	GspellChecker *checker;
	gchar *misspelled_word;

	gchar *suggested_word;
	GspellSuggestionActivatedCallback callback;
	gpointer user_data;
};

static void
suggestion_data_free (gpointer data)
{
	SuggestionData *suggestion_data = data;

	if (suggestion_data != NULL)
	{
		g_clear_object (&suggestion_data->checker);
		g_free (suggestion_data->misspelled_word);
		g_free (suggestion_data->suggested_word);
		g_free (suggestion_data);
	}
}

static void
activate_language_cb (GtkWidget *menu_item)
{
	LanguageData *data;

	data = g_object_get_data (G_OBJECT (menu_item), LANGUAGE_DATA_KEY);
	g_return_if_fail (data != NULL);

	if (data->callback != NULL)
	{
		data->callback (data->lang, data->user_data);
	}
}

static GtkWidget *
get_language_menu (const GspellLanguage            *current_language,
		   GspellLanguageActivatedCallback  callback,
		   gpointer                         user_data)
{
	GtkWidget *menu;
	const GList *languages;
	const GList *l;

	menu = gtk_menu_new ();

	languages = gspell_language_get_available ();
	for (l = languages; l != NULL; l = l->next)
	{
		const GspellLanguage *lang = l->data;
		const gchar *lang_name;
		GtkWidget *menu_item;
		LanguageData *data;

		lang_name = gspell_language_get_name (lang);

		if (lang == current_language)
		{
			/* Do not create a group. Just mark the current language
			 * as active.
			 *
			 * With a group, the first language in the list gets
			 * activated, which changes the GspellChecker language
			 * before we arrive to the current_language.
			 *
			 * Also, having a bullet only for the current_language is
			 * sufficient (to be like in Firefox), the menu is
			 * anyway ephemeral. No need to have an empty bullet for
			 * all the other languages.
			 */
			menu_item = gtk_radio_menu_item_new_with_label (NULL, lang_name);
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), TRUE);
		}
		else
		{
			menu_item = gtk_menu_item_new_with_label (lang_name);
		}

		gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

		data = g_new0 (LanguageData, 1);
		data->lang = lang;
		data->callback = callback;
		data->user_data = user_data;

		g_object_set_data_full (G_OBJECT (menu_item),
					LANGUAGE_DATA_KEY,
					data,
					g_free);

		g_signal_connect (menu_item,
				  "activate",
				  G_CALLBACK (activate_language_cb),
				  NULL);
	}

	return menu;
}

GtkMenuItem *
_gspell_context_menu_get_language_menu_item (const GspellLanguage            *current_language,
					     GspellLanguageActivatedCallback  callback,
					     gpointer                         user_data)
{
	GtkWidget *lang_menu;
	GtkMenuItem *menu_item;

	lang_menu = get_language_menu (current_language, callback, user_data);

	menu_item = GTK_MENU_ITEM (gtk_menu_item_new_with_mnemonic (_("_Language")));
	gtk_menu_item_set_submenu (menu_item, lang_menu);
	gtk_widget_show_all (GTK_WIDGET (menu_item));

	return menu_item;
}

static void
activate_suggestion_cb (GtkWidget *menu_item)
{
	SuggestionData *data;

	data = g_object_get_data (G_OBJECT (menu_item), SUGGESTION_DATA_KEY);
	g_return_if_fail (data != NULL);

	if (data->callback != NULL)
	{
		data->callback (data->suggested_word, data->user_data);
	}
}

static void
ignore_all_cb (GtkWidget *menu_item)
{
	SuggestionData *data;

	data = g_object_get_data (G_OBJECT (menu_item), SUGGESTION_DATA_KEY);
	g_return_if_fail (data != NULL);

	gspell_checker_add_word_to_session (data->checker,
					    data->misspelled_word,
					    -1);
}

static void
add_to_dictionary_cb (GtkWidget *menu_item)
{
	SuggestionData *data;

	data = g_object_get_data (G_OBJECT (menu_item), SUGGESTION_DATA_KEY);
	g_return_if_fail (data != NULL);

	gspell_checker_add_word_to_personal (data->checker,
					     data->misspelled_word,
					     -1);
}

static GtkWidget *
get_suggestion_menu (GspellChecker                     *checker,
		     const gchar                       *misspelled_word,
		     GspellSuggestionActivatedCallback  callback,
		     gpointer                           user_data)
{
	GtkWidget *top_menu;
	GtkWidget *menu_item;
	GSList *suggestions = NULL;
	SuggestionData *data;

	top_menu = gtk_menu_new ();

	suggestions = gspell_checker_get_suggestions (checker, misspelled_word, -1);

	if (suggestions == NULL)
	{
		/* No suggestions. Put something in the menu anyway... */
		menu_item = gtk_menu_item_new_with_label (_("(no suggested words)"));
		gtk_widget_set_sensitive (menu_item, FALSE);
		gtk_menu_shell_prepend (GTK_MENU_SHELL (top_menu), menu_item);
	}
	else
	{
		GtkWidget *menu = top_menu;
		gint count = 0;
		GSList *l;

		/* Build a set of menus with suggestions. */
		for (l = suggestions; l != NULL; l = l->next)
		{
			gchar *suggested_word = l->data;
			GtkWidget *label;
			gchar *label_text;

			if (count == 10)
			{
				/* Separator */
				menu_item = gtk_separator_menu_item_new ();
				gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

				menu_item = gtk_menu_item_new_with_mnemonic (_("_More…"));
				gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

				menu = gtk_menu_new ();
				gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu);
				count = 0;
			}

			label_text = g_strdup_printf ("<b>%s</b>", suggested_word);

			label = gtk_label_new (label_text);
			gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
			gtk_widget_set_halign (label, GTK_ALIGN_START);

			menu_item = gtk_menu_item_new ();
			gtk_container_add (GTK_CONTAINER (menu_item), label);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

			data = g_new0 (SuggestionData, 1);
			data->suggested_word = g_strdup (suggested_word);
			data->callback = callback;
			data->user_data = user_data;

			g_object_set_data_full (G_OBJECT (menu_item),
						SUGGESTION_DATA_KEY,
						data,
						suggestion_data_free);

			g_signal_connect (menu_item,
					  "activate",
					  G_CALLBACK (activate_suggestion_cb),
					  NULL);

			g_free (label_text);
			count++;
		}
	}

	g_slist_free_full (suggestions, g_free);

	/* Separator */
	menu_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), menu_item);

	/* Ignore all */
	menu_item = gtk_menu_item_new_with_mnemonic (_("_Ignore All"));
	gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), menu_item);

	data = g_new0 (SuggestionData, 1);
	data->checker = g_object_ref (checker);
	data->misspelled_word = g_strdup (misspelled_word);

	g_object_set_data_full (G_OBJECT (menu_item),
				SUGGESTION_DATA_KEY,
				data,
				suggestion_data_free);

	g_signal_connect (menu_item,
			  "activate",
			  G_CALLBACK (ignore_all_cb),
			  NULL);

	/* Add to Dictionary */
	menu_item = gtk_menu_item_new_with_mnemonic (_("_Add"));
	gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), menu_item);

	data = g_new0 (SuggestionData, 1);
	data->checker = g_object_ref (checker);
	data->misspelled_word = g_strdup (misspelled_word);

	g_object_set_data_full (G_OBJECT (menu_item),
				SUGGESTION_DATA_KEY,
				data,
				suggestion_data_free);

	g_signal_connect (menu_item,
			  "activate",
			  G_CALLBACK (add_to_dictionary_cb),
			  NULL);

	return top_menu;
}

GtkMenuItem *
_gspell_context_menu_get_suggestions_menu_item (GspellChecker                     *checker,
						const gchar                       *misspelled_word,
						GspellSuggestionActivatedCallback  callback,
						gpointer                           user_data)
{
	GtkWidget *suggestion_menu;
	GtkMenuItem *menu_item;

	g_return_val_if_fail (GSPELL_IS_CHECKER (checker), NULL);
	g_return_val_if_fail (misspelled_word != NULL, NULL);

	suggestion_menu = get_suggestion_menu (checker,
					       misspelled_word,
					       callback,
					       user_data);

	menu_item = GTK_MENU_ITEM (gtk_menu_item_new_with_mnemonic (_("_Spelling Suggestions…")));
	gtk_menu_item_set_submenu (menu_item, suggestion_menu);
	gtk_widget_show_all (GTK_WIDGET (menu_item));

	return menu_item;
}
