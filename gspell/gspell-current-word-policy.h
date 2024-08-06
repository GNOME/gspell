/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_CURRENT_WORD_POLICY (_gspell_current_word_policy_get_type ())

G_GNUC_INTERNAL
G_DECLARE_DERIVABLE_TYPE (GspellCurrentWordPolicy, _gspell_current_word_policy,
			  GSPELL, CURRENT_WORD_POLICY,
			  GObject)

struct _GspellCurrentWordPolicyClass
{
	GObjectClass parent_class;
};

G_GNUC_INTERNAL
GspellCurrentWordPolicy *
		_gspell_current_word_policy_new				(void);

G_GNUC_INTERNAL
gboolean	_gspell_current_word_policy_get_check_current_word	(GspellCurrentWordPolicy *policy);

G_GNUC_INTERNAL
void		_gspell_current_word_policy_set_check_current_word	(GspellCurrentWordPolicy *policy,
									 gboolean                 check_current_word);

G_GNUC_INTERNAL
void		_gspell_current_word_policy_session_cleared		(GspellCurrentWordPolicy *policy);

G_GNUC_INTERNAL
void		_gspell_current_word_policy_language_changed		(GspellCurrentWordPolicy *policy);

G_GNUC_INTERNAL
void		_gspell_current_word_policy_checker_changed		(GspellCurrentWordPolicy *policy);

G_GNUC_INTERNAL
void		_gspell_current_word_policy_cursor_moved		(GspellCurrentWordPolicy *policy);

G_GNUC_INTERNAL
void		_gspell_current_word_policy_several_chars_inserted	(GspellCurrentWordPolicy *policy);

G_GNUC_INTERNAL
void		_gspell_current_word_policy_single_char_inserted	(GspellCurrentWordPolicy *policy,
									 gunichar                 ch,
									 gboolean                 empty_selection,
									 gboolean                 at_cursor_pos);

G_GNUC_INTERNAL
void		_gspell_current_word_policy_text_deleted		(GspellCurrentWordPolicy *policy,
									 gboolean                 empty_selection,
									 gboolean                 spans_several_lines,
									 gboolean                 several_chars,
									 gboolean                 cursor_pos_at_start,
									 gboolean                 cursor_pos_at_end,
									 gboolean                 start_is_inside_word,
									 gboolean                 start_ends_word,
									 gboolean                 end_is_inside_word,
									 gboolean                 end_ends_word);

G_END_DECLS
