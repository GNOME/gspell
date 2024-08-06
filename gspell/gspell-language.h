/* SPDX-FileCopyrightText: 2006 - Paolo Maggi
 * SPDX-FileCopyrightText: 2008 - Novell, Inc.
 * SPDX-FileCopyrightText: 2015, 2016 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Based on GtkhtmlSpellLanguage (Novell), which was based on Marco Barisione's
 * GSpellLanguage, which was based on GeditSpellCheckerLanguage, which was based
 * partly on Epiphany's code.
 */

#pragma once

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
