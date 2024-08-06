/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell-config.h"
#include "gspell-text-iter.h"
#include "gspell-utils.h"

/* The same functions as the gtk_text_iter_* equivalents, but take into account:
 * - Word contractions with an apostrophe. For example "doesn't", which is a
 *   contraction of the two words "does not".
 * - Compounds with words separated by dashes. For example "spell-checking".
 *
 * When to include an apostrophe or a dash in a word? The heuristic is that the
 * apostrophe must be surrounded by a pango-defined word on *each* side of the
 * apostrophe.  In other words, there must be a word end on the left side and a
 * word start on the right side.
 *
 * Note that with that rule, a word can contain several apostrophes or dashes,
 * like "rock'n'roll". Usually such a word would be considered as misspelled,
 * but it's important to take every apostrophes, otherwise the word boundaries
 * would change depending on the GtkTextIter location, which would lead to bugs.
 *
 * Possible improvement: support words like "doin'" or "'til". That is, if the
 * "internal" word ("doin" or "til") is surrounded by only one apostrophe, take
 * the apostrophe. The implementation would be slightly more complicated, since
 * a function behavior depends on the other side of the word.
 *
 * When doing changes to the algo here, it should be reflected for the GtkEntry
 * support as well, to have a consistent behavior.
 *
 * TODO: the following Pango bug is now mostly done, see if the gtk_text_iter_*
 * functions can be used directly, or if the code here can be simplified.
 * https://bugzilla.gnome.org/show_bug.cgi?id=97545
 * "Make pango_default_break follow Unicode TR #29"
 */

static gboolean
is_apostrophe_or_dash (const GtkTextIter *iter)
{
	gunichar ch;

	ch = gtk_text_iter_get_char (iter);

	return _gspell_utils_is_apostrophe_or_dash (ch);
}

gboolean
_gspell_text_iter_forward_word_end (GtkTextIter *iter)
{
	g_return_val_if_fail (iter != NULL, FALSE);

	while (gtk_text_iter_forward_word_end (iter))
	{
		GtkTextIter next_char;

		if (!is_apostrophe_or_dash (iter))
		{
			return TRUE;
		}

		next_char = *iter;
		gtk_text_iter_forward_char (&next_char);

		if (!gtk_text_iter_starts_word (&next_char))
		{
			return TRUE;
		}

		*iter = next_char;
	}

	return FALSE;
}

gboolean
_gspell_text_iter_backward_word_start (GtkTextIter *iter)
{
	g_return_val_if_fail (iter != NULL, FALSE);

	while (gtk_text_iter_backward_word_start (iter))
	{
		GtkTextIter prev_char = *iter;

		if (!gtk_text_iter_backward_char (&prev_char) ||
		    !is_apostrophe_or_dash (&prev_char) ||
		    !gtk_text_iter_ends_word (&prev_char))
		{
			return TRUE;
		}

		*iter = prev_char;
	}

	return FALSE;
}

gboolean
_gspell_text_iter_starts_word (const GtkTextIter *iter)
{
	GtkTextIter prev_char;

	g_return_val_if_fail (iter != NULL, FALSE);

	if (!gtk_text_iter_starts_word (iter))
	{
		return FALSE;
	}

	prev_char = *iter;
	if (!gtk_text_iter_backward_char (&prev_char))
	{
		return TRUE;
	}

	if (is_apostrophe_or_dash (&prev_char) &&
	    gtk_text_iter_ends_word (&prev_char))
	{
		return FALSE;
	}

	return TRUE;
}

gboolean
_gspell_text_iter_ends_word (const GtkTextIter *iter)
{
	GtkTextIter next_char;

	g_return_val_if_fail (iter != NULL, FALSE);

	if (!gtk_text_iter_ends_word (iter))
	{
		return FALSE;
	}

	if (gtk_text_iter_is_end (iter))
	{
		return TRUE;
	}

	next_char = *iter;
	gtk_text_iter_forward_char (&next_char);

	if (is_apostrophe_or_dash (iter) &&
	    gtk_text_iter_starts_word (&next_char))
	{
		return FALSE;
	}

	return TRUE;
}

gboolean
_gspell_text_iter_inside_word (const GtkTextIter *iter)
{
	g_return_val_if_fail (iter != NULL, FALSE);

	if (gtk_text_iter_inside_word (iter))
	{
		return TRUE;
	}

	if (gtk_text_iter_ends_word (iter) &&
	    is_apostrophe_or_dash (iter))
	{
		GtkTextIter next_char = *iter;
		gtk_text_iter_forward_char (&next_char);
		return gtk_text_iter_starts_word (&next_char);
	}

	return FALSE;
}
