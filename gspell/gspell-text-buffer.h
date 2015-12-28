/*
 * This file is part of gspell, a spell-checking library.
 *
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

#ifndef __GSPELL_TEXT_BUFFER_H__
#define __GSPELL_TEXT_BUFFER_H__

#if !defined (__GSPELL_H_INSIDE__) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <gspell/gspell-checker.h>
#include <gtk/gtk.h>

void		gspell_text_buffer_set_spell_checker	(GtkTextBuffer *buffer,
							 GspellChecker *checker);

GspellChecker *	gspell_text_buffer_get_spell_checker	(GtkTextBuffer *buffer);

#endif /* __GSPELL_TEXT_BUFFER_H__ */

/* ex:set ts=8 noet: */
