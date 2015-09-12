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

#include "gspell-spell-navigator.h"

/**
 * SECTION:spell-navigator
 * @Short_description: Interface to navigate through misspelled words
 * @Title: GspellSpellNavigator
 * @See_also: #GspellSpellNavigatorGtv, #GspellSpellCheckerDialog
 *
 * #GspellSpellNavigator is an interface to navigate through misspelled words,
 * and correct the mistakes.
 *
 * It is used by widgets like #GspellSpellCheckerDialog. The purpose is to
 * spell check a document one word at a time.
 *
 * It is not mandatory to navigate through all the text. Depending on the
 * context, an implementation could spell check only the current page, or the
 * selection, etc.
 */

G_DEFINE_INTERFACE (GspellSpellNavigator, gspell_spell_navigator, G_TYPE_OBJECT)

static gboolean
gspell_spell_navigator_goto_next_default (GspellSpellNavigator  *navigator,
					  gchar               **word,
					  GspellSpellChecker   **spell_checker,
					  GError              **error)
{
	return FALSE;
}

static void
gspell_spell_navigator_change_default (GspellSpellNavigator *navigator,
				       const gchar         *word,
				       const gchar         *change_to)
{
}

static void
gspell_spell_navigator_change_all_default (GspellSpellNavigator *navigator,
					   const gchar         *word,
					   const gchar         *change_to)
{
}

static void
gspell_spell_navigator_default_init (GspellSpellNavigatorInterface *iface)
{
	iface->goto_next = gspell_spell_navigator_goto_next_default;
	iface->change = gspell_spell_navigator_change_default;
	iface->change_all = gspell_spell_navigator_change_all_default;
}

/**
 * gspell_spell_navigator_goto_next:
 * @navigator: a #GspellSpellNavigator.
 * @word: (out) (optional): a location to store an allocated string, or %NULL.
 *   Use g_free() to free the returned string.
 * @spell_checker: (out) (optional) (transfer full): a location to store the
 *   #GspellSpellChecker used, or %NULL. Use g_object_unref() when no longer
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
gspell_spell_navigator_goto_next (GspellSpellNavigator  *navigator,
				  gchar               **word,
				  GspellSpellChecker   **spell_checker,
				  GError              **error)
{
	g_return_val_if_fail (GSPELL_IS_SPELL_NAVIGATOR (navigator), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if (word != NULL)
	{
		*word = NULL;
	}

	if (spell_checker != NULL)
	{
		*spell_checker = NULL;
	}

	return GSPELL_SPELL_NAVIGATOR_GET_IFACE (navigator)->goto_next (navigator,
								       word,
								       spell_checker,
								       error);
}

/**
 * gspell_spell_navigator_change:
 * @navigator: a #GspellSpellNavigator.
 * @word: the word to change.
 * @change_to: the replacement.
 *
 * Changes the current @word by @change_to. @word is the same as returned by the
 * last gspell_spell_navigator_goto_next().
 *
 * gspell_spell_checker_set_correction() has already been called, this function
 * is to make the change in the text. Only the current word should be changed.
 */
void
gspell_spell_navigator_change (GspellSpellNavigator *navigator,
			       const gchar         *word,
			       const gchar         *change_to)
{
	g_return_if_fail (GSPELL_IS_SPELL_NAVIGATOR (navigator));

	GSPELL_SPELL_NAVIGATOR_GET_IFACE (navigator)->change (navigator, word, change_to);
}

/**
 * gspell_spell_navigator_change_all:
 * @navigator: a #GspellSpellNavigator.
 * @word: the word to change.
 * @change_to: the replacement.
 *
 * Changes all occurrences of @word by @change_to.
 *
 * gspell_spell_checker_set_correction() has already been called, this function
 * is to make the change in the text.
 */
void
gspell_spell_navigator_change_all (GspellSpellNavigator *navigator,
				   const gchar         *word,
				   const gchar         *change_to)
{
	g_return_if_fail (GSPELL_IS_SPELL_NAVIGATOR (navigator));

	GSPELL_SPELL_NAVIGATOR_GET_IFACE (navigator)->change_all (navigator, word, change_to);
}

/* ex:set ts=8 noet: */
