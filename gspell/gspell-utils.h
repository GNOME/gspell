/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2010 - Jesse van den Kieboom
 * Copyright 2015, 2016, 2017 - Sébastien Wilmet
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

#ifndef GSPELL_UTILS_H
#define GSPELL_UTILS_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

/* gunichar decimal value of unicode apostrophe characters. */
#define _GSPELL_MODIFIER_LETTER_APOSTROPHE (700) /* U+02BC */
#define _GSPELL_RIGHT_SINGLE_QUOTATION_MARK (8217) /* U+2019 */

G_GNUC_INTERNAL
gboolean	_gspell_utils_is_number			(const gchar *text,
							 gssize       text_length);

G_GNUC_INTERNAL
GtkTextTag *	_gspell_utils_get_no_spell_check_tag	(GtkTextBuffer *buffer);

G_GNUC_INTERNAL
gboolean	_gspell_utils_skip_no_spell_check	(GtkTextTag        *no_spell_check_tag,
							 GtkTextIter       *start,
							 const GtkTextIter *end);

G_GNUC_INTERNAL
gchar *		_gspell_utils_str_replace		(const gchar *string,
							 const gchar *search,
							 const gchar *replacement);

G_GNUC_INTERNAL
gboolean	_gspell_utils_str_to_ascii_apostrophe	(const gchar  *word,
							 gssize        word_length,
							 gchar       **result);

G_GNUC_INTERNAL
gboolean	_gspell_utils_is_apostrophe_or_dash	(gunichar ch);

G_GNUC_INTERNAL
void		_gspell_utils_init_underline_rgba	(GdkRGBA *underline_color);

G_GNUC_INTERNAL
PangoAttribute *_gspell_utils_create_pango_attr_underline_color (void);

G_GNUC_INTERNAL
void		_gspell_utils_improve_word_boundaries	(const gchar  *text,
							 PangoLogAttr *log_attrs,
							 gint          n_attrs);

G_END_DECLS

#endif /* GSPELL_UTILS_H */

/* ex:set ts=8 noet: */
