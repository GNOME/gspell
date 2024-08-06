/* SPDX-FileCopyrightText: 2002 - Paolo Maggi
 * SPDX-FileCopyrightText: 2015 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* This is a modified version of GtkSpell 2.0.2 (gtkspell.sf.net)
 * SPDX-FileCopyrightText: 2002 - Evan Martin
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_INLINE_CHECKER_TEXT_BUFFER (_gspell_inline_checker_text_buffer_get_type ())

G_GNUC_INTERNAL
G_DECLARE_FINAL_TYPE (GspellInlineCheckerTextBuffer, _gspell_inline_checker_text_buffer,
		      GSPELL, INLINE_CHECKER_TEXT_BUFFER,
		      GObject)

G_GNUC_INTERNAL
GspellInlineCheckerTextBuffer *
	_gspell_inline_checker_text_buffer_new			(GtkTextBuffer *buffer);

G_GNUC_INTERNAL
void	_gspell_inline_checker_text_buffer_attach_view		(GspellInlineCheckerTextBuffer *spell,
								 GtkTextView                   *view);

G_GNUC_INTERNAL
void	_gspell_inline_checker_text_buffer_detach_view		(GspellInlineCheckerTextBuffer *spell,
								 GtkTextView                   *view);

G_GNUC_INTERNAL
void	_gspell_inline_checker_text_buffer_populate_popup	(GspellInlineCheckerTextBuffer *spell,
								 GtkMenu                       *menu);

/* For unit tests */

G_GNUC_INTERNAL
void	_gspell_inline_checker_text_buffer_set_unit_test_mode	(GspellInlineCheckerTextBuffer *spell,
								 gboolean                       unit_test_mode);

G_GNUC_INTERNAL
GtkTextTag *
	_gspell_inline_checker_text_buffer_get_highlight_tag	(GspellInlineCheckerTextBuffer *spell);

G_END_DECLS
