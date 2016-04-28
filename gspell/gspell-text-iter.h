/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef GSPELL_TEXT_ITER_H
#define GSPELL_TEXT_ITER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

G_GNUC_INTERNAL
gboolean	_gspell_text_iter_forward_word_end	(GtkTextIter *iter);

G_GNUC_INTERNAL
gboolean	_gspell_text_iter_backward_word_start	(GtkTextIter *iter);

G_GNUC_INTERNAL
gboolean	_gspell_text_iter_starts_word		(const GtkTextIter *iter);

G_GNUC_INTERNAL
gboolean	_gspell_text_iter_ends_word		(const GtkTextIter *iter);

G_GNUC_INTERNAL
gboolean	_gspell_text_iter_inside_word		(const GtkTextIter *iter);

G_END_DECLS

#endif /* GSPELL_TEXT_ITER_H */

/* ex:set ts=8 noet: */
