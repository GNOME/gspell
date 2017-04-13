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

#ifndef GSPELL_CURRENT_WORD_POLICY_H
#define GSPELL_CURRENT_WORD_POLICY_H

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

#endif /* GSPELL_CURRENT_WORD_POLICY_H */

/* ex:set ts=8 noet: */
