/* SPDX-FileCopyrightText: 2010 - Jesse van den Kieboom
 * SPDX-FileCopyrightText: 2015, 2016, 2017 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

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
