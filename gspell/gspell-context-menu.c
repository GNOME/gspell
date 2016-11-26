/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2016 - Sébastien Wilmet
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
#include "gspell-context-menu.h"
#include <glib/gi18n-lib.h>

#define LANGUAGE_DATA_KEY "gspell-language-data-key"

typedef struct _LanguageData LanguageData;
struct _LanguageData
{
	const GspellLanguage *lang;
	GspellLanguageActivatedCallback callback;
	gpointer user_data;
};

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

/* ex:set ts=8 noet: */
