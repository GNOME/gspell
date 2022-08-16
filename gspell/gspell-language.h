/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2006 - Paolo Maggi
 * Copyright 2008 - Novell, Inc.
 * Copyright 2015, 2016 - Sébastien Wilmet
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

/* Based on GtkhtmlSpellLanguage (Novell), which was based on Marco Barisione's
 * GSpellLanguage, which was based on GeditSpellCheckerLanguage, which was based
 * partly on Epiphany's code.
 */

#ifndef GSPELL_LANGUAGE_H
#define GSPELL_LANGUAGE_H

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <glib-object.h>
#include <gmodule.h>

G_BEGIN_DECLS

typedef struct _GspellLanguage GspellLanguage;

#define GSPELL_TYPE_LANGUAGE (gspell_language_get_type ())

G_MODULE_EXPORT
GType		gspell_language_get_type		(void) G_GNUC_CONST;

G_MODULE_EXPORT
const GList *	gspell_language_get_available		(void);

G_MODULE_EXPORT
const GspellLanguage *
		gspell_language_get_default		(void);

G_MODULE_EXPORT
const GspellLanguage *
		gspell_language_lookup			(const gchar *language_code);

G_MODULE_EXPORT
const gchar *	gspell_language_get_code		(const GspellLanguage *language);

G_MODULE_EXPORT
const gchar *	gspell_language_get_name		(const GspellLanguage *language);

G_MODULE_EXPORT
gint		gspell_language_compare			(const GspellLanguage *language_a,
							 const GspellLanguage *language_b);

G_MODULE_EXPORT
GspellLanguage *gspell_language_copy			(const GspellLanguage *language);

G_MODULE_EXPORT
void		gspell_language_free			(GspellLanguage *language);

G_END_DECLS

#endif /* GSPELL_LANGUAGE_H */
