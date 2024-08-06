/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell-config.h"
#include "gspell-current-word-policy.h"

/* An object that decides whether to check the current word. When a word is
 * being typed, it should not be spell-checked, because it would be annoying to
 * see the red wavy underline appearing and disappearing constantly.
 *
 * You need to feed the object with events, and get the result with
 * _gspell_current_word_policy_get_check_current_word().
 */

typedef struct _GspellCurrentWordPolicyPrivate GspellCurrentWordPolicyPrivate;

struct _GspellCurrentWordPolicyPrivate
{
	guint check_current_word : 1;
};

G_DEFINE_TYPE_WITH_PRIVATE (GspellCurrentWordPolicy, _gspell_current_word_policy, G_TYPE_OBJECT)

static void
_gspell_current_word_policy_dispose (GObject *object)
{

	G_OBJECT_CLASS (_gspell_current_word_policy_parent_class)->dispose (object);
}

static void
_gspell_current_word_policy_finalize (GObject *object)
{

	G_OBJECT_CLASS (_gspell_current_word_policy_parent_class)->finalize (object);
}

static void
_gspell_current_word_policy_class_init (GspellCurrentWordPolicyClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->dispose = _gspell_current_word_policy_dispose;
	object_class->finalize = _gspell_current_word_policy_finalize;
}

static void
_gspell_current_word_policy_init (GspellCurrentWordPolicy *policy)
{
	GspellCurrentWordPolicyPrivate *priv;

	priv = _gspell_current_word_policy_get_instance_private (policy);
	priv->check_current_word = TRUE;
}

GspellCurrentWordPolicy *
_gspell_current_word_policy_new (void)
{
	return g_object_new (GSPELL_TYPE_CURRENT_WORD_POLICY, NULL);
}

gboolean
_gspell_current_word_policy_get_check_current_word (GspellCurrentWordPolicy *policy)
{
	GspellCurrentWordPolicyPrivate *priv;

	g_return_val_if_fail (GSPELL_IS_CURRENT_WORD_POLICY (policy), TRUE);

	priv = _gspell_current_word_policy_get_instance_private (policy);

	return priv->check_current_word;
}

/* For other events, it's better to use the more specific feed functions if
 * possible.
 */
void
_gspell_current_word_policy_set_check_current_word (GspellCurrentWordPolicy *policy,
						    gboolean                 check_current_word)
{
	GspellCurrentWordPolicyPrivate *priv;

	g_return_if_fail (GSPELL_IS_CURRENT_WORD_POLICY (policy));

	priv = _gspell_current_word_policy_get_instance_private (policy);

	priv->check_current_word = check_current_word != FALSE;
}

/* On GspellChecker::session-cleared signal. */
void
_gspell_current_word_policy_session_cleared (GspellCurrentWordPolicy *policy)
{
	g_return_if_fail (GSPELL_IS_CURRENT_WORD_POLICY (policy));

	_gspell_current_word_policy_set_check_current_word (policy, TRUE);
}

/* On GspellChecker::notify::language signal. */
void
_gspell_current_word_policy_language_changed (GspellCurrentWordPolicy *policy)
{
	g_return_if_fail (GSPELL_IS_CURRENT_WORD_POLICY (policy));

	_gspell_current_word_policy_set_check_current_word (policy, TRUE);
}

/* When another GspellChecker object is used. */
void
_gspell_current_word_policy_checker_changed (GspellCurrentWordPolicy *policy)
{
	g_return_if_fail (GSPELL_IS_CURRENT_WORD_POLICY (policy));

	_gspell_current_word_policy_set_check_current_word (policy, TRUE);
}

void
_gspell_current_word_policy_cursor_moved (GspellCurrentWordPolicy *policy)
{
	g_return_if_fail (GSPELL_IS_CURRENT_WORD_POLICY (policy));

	_gspell_current_word_policy_set_check_current_word (policy, TRUE);
}

/* After a text insertion. */
void
_gspell_current_word_policy_several_chars_inserted (GspellCurrentWordPolicy *policy)
{
	g_return_if_fail (GSPELL_IS_CURRENT_WORD_POLICY (policy));

	/* If more than one character is inserted, it's probably not a normal
	 * keypress, e.g. a clipboard paste or DND. So it's better to check the
	 * current word in that case, to know ASAP if the word is correctly
	 * spelled.
	 */
	_gspell_current_word_policy_set_check_current_word (policy, TRUE);
}

/* After a text insertion. */
void
_gspell_current_word_policy_single_char_inserted (GspellCurrentWordPolicy *policy,
						  gunichar                 ch,
						  gboolean                 empty_selection,
						  gboolean                 at_cursor_pos)
{
	g_return_if_fail (GSPELL_IS_CURRENT_WORD_POLICY (policy));

	/* If e.g. a space or punctuation is inserted, we want to check the
	 * current word, since in that case we are not editing the current word.
	 * Maybe a word has been split in two, in which case the word on the
	 * left will anyway be checked, so it's better to know directly whether
	 * the word on the right is correctly spelled as well, so we know if we
	 * need to edit it or not.
	 * If there is a selection, it means that the text was inserted
	 * programmatically, so the user is not editing the current word
	 * manually.
	 */
	if (g_unichar_isalnum (ch) &&
	    empty_selection &&
	    at_cursor_pos)
	{
		_gspell_current_word_policy_set_check_current_word (policy, FALSE);
	}
	else
	{
		_gspell_current_word_policy_set_check_current_word (policy, TRUE);
	}
}

/* Before a text deletion.
 *
 * "start" refers to the start of the deletion.
 * "end" refers to the end of the deletion.
 * It is assumed that start < end.
 *
 * "inside word" and "ends word" have the same semantics as
 * gtk_text_iter_inside_word() and gtk_text_iter_ends_word(), but custom word
 * boundaries can be used.
 */
void
_gspell_current_word_policy_text_deleted (GspellCurrentWordPolicy *policy,
					  gboolean                 empty_selection,
					  gboolean                 spans_several_lines,
					  gboolean                 several_chars,
					  gboolean                 cursor_pos_at_start,
					  gboolean                 cursor_pos_at_end,
					  gboolean                 start_is_inside_word,
					  gboolean                 start_ends_word,
					  gboolean                 end_is_inside_word,
					  gboolean                 end_ends_word)
{
	g_return_if_fail (GSPELL_IS_CURRENT_WORD_POLICY (policy));

	if (!empty_selection ||
	    spans_several_lines ||
	    several_chars)
	{
		_gspell_current_word_policy_set_check_current_word (policy, TRUE);
	}
	/* Probably backspace key */
	else if (cursor_pos_at_end)
	{
		if (start_is_inside_word || start_ends_word)
		{
			_gspell_current_word_policy_set_check_current_word (policy, FALSE);
		}
		else
		{
			_gspell_current_word_policy_set_check_current_word (policy, TRUE);
		}
	}
	/* Probably delete key */
	else if (cursor_pos_at_start)
	{
		if (end_is_inside_word || end_ends_word)
		{
			_gspell_current_word_policy_set_check_current_word (policy, FALSE);
		}
		else
		{
			_gspell_current_word_policy_set_check_current_word (policy, TRUE);
		}
	}
	/* Text deleted programmatically */
	else
	{
		_gspell_current_word_policy_set_check_current_word (policy, TRUE);
	}
}
