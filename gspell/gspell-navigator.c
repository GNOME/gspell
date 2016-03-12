/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2015, 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "gspell-navigator.h"

/**
 * SECTION:navigator
 * @Short_description: Interface to navigate through misspelled words
 * @Title: GspellNavigator
 * @See_also: #GspellNavigatorTextView, #GspellCheckerDialog
 *
 * #GspellNavigator is an interface to navigate through misspelled words,
 * and correct the mistakes.
 *
 * It is used by widgets like #GspellCheckerDialog. The purpose is to
 * spell check a document one word at a time.
 *
 * It is not mandatory to navigate through all the text. Depending on the
 * context, an implementation could spell check only the current page, or the
 * selection, etc.
 *
 * For #GtkTextView, see the #GspellNavigatorTextView implementation of this
 * interface.
 */

G_DEFINE_INTERFACE (GspellNavigator, gspell_navigator, G_TYPE_INITIALLY_UNOWNED)

static gboolean
gspell_navigator_goto_next_default (GspellNavigator  *navigator,
				    gchar           **word,
				    GspellChecker   **spell_checker,
				    GError          **error)
{
	return FALSE;
}

static void
gspell_navigator_change_default (GspellNavigator *navigator,
				 const gchar     *word,
				 const gchar     *change_to)
{
}

static void
gspell_navigator_change_all_default (GspellNavigator *navigator,
				     const gchar     *word,
				     const gchar     *change_to)
{
}

static void
gspell_navigator_default_init (GspellNavigatorInterface *iface)
{
	iface->goto_next = gspell_navigator_goto_next_default;
	iface->change = gspell_navigator_change_default;
	iface->change_all = gspell_navigator_change_all_default;
}

/**
 * gspell_navigator_goto_next:
 * @navigator: a #GspellNavigator.
 * @word: (out) (optional): a location to store an allocated string, or %NULL.
 *   Use g_free() to free the returned string.
 * @spell_checker: (out) (optional) (transfer full): a location to store the
 *   #GspellChecker used, or %NULL. Use g_object_unref() when no longer
 *   needed.
 * @error: (out) (optional): a location to a %NULL #GError, or %NULL.
 *
 * Goes to the next misspelled word. When called the first time, goes to the
 * first misspelled word.
 *
 * Returns: %TRUE if a next misspelled word has been found, %FALSE if the spell
 * checking is finished or if an error occurred.
 */
gboolean
gspell_navigator_goto_next (GspellNavigator  *navigator,
			    gchar           **word,
			    GspellChecker   **spell_checker,
			    GError          **error)
{
	g_return_val_if_fail (GSPELL_IS_NAVIGATOR (navigator), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if (word != NULL)
	{
		*word = NULL;
	}

	if (spell_checker != NULL)
	{
		*spell_checker = NULL;
	}

	return GSPELL_NAVIGATOR_GET_IFACE (navigator)->goto_next (navigator,
								  word,
								  spell_checker,
								  error);
}

/**
 * gspell_navigator_change:
 * @navigator: a #GspellNavigator.
 * @word: the word to change.
 * @change_to: the replacement.
 *
 * Changes the current @word by @change_to in the text. @word must be the same
 * as returned by the last call to gspell_navigator_goto_next().
 *
 * This function doesn't call gspell_checker_set_correction(). A widget using a
 * #GspellNavigator should call gspell_checker_set_correction() in addition to
 * this function.
 */
void
gspell_navigator_change (GspellNavigator *navigator,
			 const gchar     *word,
			 const gchar     *change_to)
{
	g_return_if_fail (GSPELL_IS_NAVIGATOR (navigator));

	GSPELL_NAVIGATOR_GET_IFACE (navigator)->change (navigator, word, change_to);
}

/**
 * gspell_navigator_change_all:
 * @navigator: a #GspellNavigator.
 * @word: the word to change.
 * @change_to: the replacement.
 *
 * Changes all occurrences of @word by @change_to in the text.
 *
 * This function doesn't call gspell_checker_set_correction(). A widget using a
 * #GspellNavigator should call gspell_checker_set_correction() in addition to
 * this function.
 */
void
gspell_navigator_change_all (GspellNavigator *navigator,
			     const gchar     *word,
			     const gchar     *change_to)
{
	g_return_if_fail (GSPELL_IS_NAVIGATOR (navigator));

	GSPELL_NAVIGATOR_GET_IFACE (navigator)->change_all (navigator, word, change_to);
}

/* ex:set ts=8 noet: */
