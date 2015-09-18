/*
 * This file is part of gspell.
 *
 * Copyright 2015 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "gspell-language-chooser.h"

/**
 * SECTION:language-chooser
 * @Short_description: Interface to choose a GspellLanguage
 * @Title: GspellLanguageChooser
 * @See_also: #GspellLanguage, #GspellLanguageChooserButton, #GspellLanguageChooserDialog
 *
 * #GspellLanguageChooser is an interface that is implemented by widgets for
 * choosing a #GspellLanguage.
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
					     g_param_spec_pointer ("language",
								   "Language",
								   "",
								   G_PARAM_READWRITE |
								   G_PARAM_STATIC_STRINGS));
}

/**
 * gspell_language_chooser_get_language:
 * @chooser: a #GspellLanguageChooser.
 *
 * Returns: (nullable): the selected #GspellLanguage.
 */
const GspellLanguage *
gspell_language_chooser_get_language (GspellLanguageChooser *chooser)
{
	g_return_val_if_fail (GSPELL_IS_LANGUAGE_CHOOSER (chooser), NULL);

	return GSPELL_LANGUAGE_CHOOSER_GET_IFACE (chooser)->get_language (chooser);
}

/**
 * gspell_language_chooser_set_language:
 * @chooser: a #GspellLanguageChooser.
 * @language: a #GspellLanguage.
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

/* ex:set ts=8 noet: */
