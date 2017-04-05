/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2015 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef GSPELL_LANGUAGE_CHOOSER_H
#define GSPELL_LANGUAGE_CHOOSER_H

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <glib-object.h>
#include <gspell/gspell-language.h>
#include <gspell/gspell-version.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_LANGUAGE_CHOOSER (gspell_language_chooser_get_type ())

GSPELL_AVAILABLE_IN_ALL
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

GSPELL_AVAILABLE_IN_ALL
const GspellLanguage *	gspell_language_chooser_get_language		(GspellLanguageChooser *chooser);

GSPELL_AVAILABLE_IN_ALL
void			gspell_language_chooser_set_language		(GspellLanguageChooser *chooser,
									 const GspellLanguage  *language);

GSPELL_AVAILABLE_IN_ALL
const gchar *		gspell_language_chooser_get_language_code	(GspellLanguageChooser *chooser);

GSPELL_AVAILABLE_IN_ALL
void			gspell_language_chooser_set_language_code	(GspellLanguageChooser *chooser,
									 const gchar           *language_code);

G_END_DECLS

#endif /* GSPELL_LANGUAGE_CHOOSER_H */

/* ex:set ts=8 noet: */
