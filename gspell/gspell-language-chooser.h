/* SPDX-FileCopyrightText: 2015 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <glib-object.h>
#include <gspell/gspell-language.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_LANGUAGE_CHOOSER (gspell_language_chooser_get_type ())

G_MODULE_EXPORT
G_DECLARE_INTERFACE (GspellLanguageChooser, gspell_language_chooser,
		     GSPELL, LANGUAGE_CHOOSER,
		     GObject)

struct _GspellLanguageChooserInterface
{
	GTypeInterface parent_interface;

	/* The return value is the same as get_language(), but there is the
	 * (optional) out parameter @default_language in addition.
	 */
	const GspellLanguage *	(* get_language_full)	(GspellLanguageChooser *chooser,
							 gboolean              *default_language);

	void			(* set_language)	(GspellLanguageChooser *chooser,
							 const GspellLanguage  *language);
};

G_MODULE_EXPORT
const GspellLanguage *	gspell_language_chooser_get_language		(GspellLanguageChooser *chooser);

G_MODULE_EXPORT
void			gspell_language_chooser_set_language		(GspellLanguageChooser *chooser,
									 const GspellLanguage  *language);

G_MODULE_EXPORT
const gchar *		gspell_language_chooser_get_language_code	(GspellLanguageChooser *chooser);

G_MODULE_EXPORT
void			gspell_language_chooser_set_language_code	(GspellLanguageChooser *chooser,
									 const gchar           *language_code);

G_END_DECLS
