/*
 * This file is part of gspell.
 *
 * Copyright 2002 - Paolo Maggi
 * Copyright 2015 - Sébastien Wilmet
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

/* This is a modified version of GtkSpell 2.0.2 (gtkspell.sf.net)
 * Copyright 2002 - Evan Martin
 */

#ifndef __GSPELL_INLINE_CHECKER_GTV_H__
#define __GSPELL_INLINE_CHECKER_GTV_H__

#if !defined (__GSPELL_H_INSIDE__) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gtksourceview/gtksource.h>
#include <gspell/gspell-checker.h>

#define GSPELL_TYPE_INLINE_CHECKER_GTV (gspell_inline_checker_gtv_get_type ())
G_DECLARE_FINAL_TYPE (GspellInlineCheckerGtv, gspell_inline_checker_gtv,
		      GSPELL, INLINE_CHECKER_GTV,
		      GObject)

GspellInlineCheckerGtv *
	gspell_inline_checker_gtv_new		(GtkSourceBuffer *buffer,
						 GspellChecker   *checker);

void	gspell_inline_checker_gtv_attach_view	(GspellInlineCheckerGtv *spell,
						 GtkTextView            *view);

void	gspell_inline_checker_gtv_detach_view	(GspellInlineCheckerGtv *spell,
						 GtkTextView            *view);

#endif  /* __GSPELL_INLINE_CHECKER_GTV_H__ */

/* ex:set ts=8 noet: */
