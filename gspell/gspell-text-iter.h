/* SPDX-FileCopyrightText: 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
