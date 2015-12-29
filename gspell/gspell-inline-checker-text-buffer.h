/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2002 - Paolo Maggi
 * Copyright 2015 - Sébastien Wilmet
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

/* This is a modified version of GtkSpell 2.0.2 (gtkspell.sf.net)
 * Copyright 2002 - Evan Martin
 */

#ifndef __GSPELL_INLINE_CHECKER_TEXT_BUFFER_H__
#define __GSPELL_INLINE_CHECKER_TEXT_BUFFER_H__

#include <gtk/gtk.h>

#define GSPELL_TYPE_INLINE_CHECKER_TEXT_BUFFER (_gspell_inline_checker_text_buffer_get_type ())

G_GNUC_INTERNAL
G_DECLARE_FINAL_TYPE (GspellInlineCheckerTextBuffer, _gspell_inline_checker_text_buffer,
		      GSPELL, INLINE_CHECKER_TEXT_BUFFER,
		      GObject)

G_GNUC_INTERNAL
GspellInlineCheckerTextBuffer *
	_gspell_inline_checker_text_buffer_new		(GtkTextBuffer *buffer);

G_GNUC_INTERNAL
void	_gspell_inline_checker_text_buffer_attach_view	(GspellInlineCheckerTextBuffer *spell,
							 GtkTextView                   *view);

G_GNUC_INTERNAL
void	_gspell_inline_checker_text_buffer_detach_view	(GspellInlineCheckerTextBuffer *spell,
							 GtkTextView                   *view);

#endif  /* __GSPELL_INLINE_CHECKER_TEXT_BUFFER_H__ */

/* ex:set ts=8 noet: */
