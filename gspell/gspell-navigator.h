/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2015, 2016 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef GSPELL_NAVIGATOR_H
#define GSPELL_NAVIGATOR_H

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <glib-object.h>
#include <gspell/gspell-checker.h>
#include <gspell/gspell-version.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_NAVIGATOR (gspell_navigator_get_type ())

GSPELL_AVAILABLE_IN_ALL
G_DECLARE_INTERFACE (GspellNavigator, gspell_navigator,
		     GSPELL, NAVIGATOR,
		     GInitiallyUnowned)

struct _GspellNavigatorInterface
{
	GTypeInterface parent_interface;

	gboolean	(* goto_next)		(GspellNavigator  *navigator,
						 gchar           **word,
						 GspellChecker   **spell_checker,
						 GError          **error);

	void		(* change)		(GspellNavigator *navigator,
						 const gchar     *word,
						 const gchar     *change_to);

	void		(* change_all)		(GspellNavigator *navigator,
						 const gchar     *word,
						 const gchar     *change_to);
};

GSPELL_AVAILABLE_IN_ALL
gboolean	gspell_navigator_goto_next	(GspellNavigator  *navigator,
						 gchar           **word,
						 GspellChecker   **spell_checker,
						 GError          **error);

GSPELL_AVAILABLE_IN_ALL
void		gspell_navigator_change		(GspellNavigator *navigator,
						 const gchar     *word,
						 const gchar     *change_to);

GSPELL_AVAILABLE_IN_ALL
void		gspell_navigator_change_all	(GspellNavigator *navigator,
						 const gchar     *word,
						 const gchar     *change_to);

G_END_DECLS

#endif /* GSPELL_NAVIGATOR_H */

/* ex:set ts=8 noet: */
