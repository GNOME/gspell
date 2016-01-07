/*
 * This file is part of gspell, a spell-checking library.
 *
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

#include "gspell-text-view.h"
#include "gspell-inline-checker-text.h"

/**
 * SECTION:text-view
 * @Title: GtkTextView support
 *
 * Spell checking support for #GtkTextView.
 */

#define INLINE_CHECKER_KEY "gspell-text-view-inline-checker-key"

/**
 * gspell_text_view_set_inline_checking:
 * @view: a #GtkTextView.
 * @enable: whether to enable the inline spell checking.
 *
 * Enables or disables inline spell checking.
 *
 * If enabled, misspelled words are highlighted with a %PANGO_UNDERLINE_ERROR,
 * usually a red wavy underline. Right-clicking a misspelled word pops up a
 * context menu of suggested replacements. The context menu also contains an
 * “Ignore All” item to add the misspelled word to the session dictionary. And
 * an “Add” item to add the word to the personal dictionary.
 *
 * The spell is checked only on the visible region of the #GtkTextView. Note
 * that if a same #GtkTextBuffer is used for several views, the misspelled words
 * are visible in all views, because the highlighting is achieved with a
 * #GtkTextTag added to the buffer.
 *
 * You need to call gspell_text_buffer_set_spell_checker() to associate a
 * #GspellChecker to the #GtkTextBuffer. #GtkTextView:buffer changes are
 * handled, as well as #GspellChecker changes.
 */
void
gspell_text_view_set_inline_checking (GtkTextView *view,
				      gboolean     enable)
{
	g_return_if_fail (GTK_IS_TEXT_VIEW (view));

	if (enable)
	{
		if (gspell_text_view_get_inline_checking (view))
		{
			return;
		}

		g_object_set_data_full (G_OBJECT (view),
					INLINE_CHECKER_KEY,
					gspell_inline_checker_text_new (view),
					g_object_unref);
	}
	else
	{
		g_object_set_data (G_OBJECT (view),
				   INLINE_CHECKER_KEY,
				   NULL);
	}
}

/**
 * gspell_text_view_get_inline_checking:
 * @view: a #GtkTextView.
 *
 * Returns: whether the inline spell checking is enabled for @view.
 */
gboolean
gspell_text_view_get_inline_checking (GtkTextView *view)
{
	g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), FALSE);

	return g_object_get_data (G_OBJECT (view), INLINE_CHECKER_KEY) != NULL;
}

/* ex:set ts=8 noet: */
