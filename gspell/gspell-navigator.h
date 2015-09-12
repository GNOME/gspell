/*
 * This file is part of gspell.
 *
 * Copyright 2015 - Sébastien Wilmet <swilmet@gnome.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __GSPELL_NAVIGATOR_H__
#define __GSPELL_NAVIGATOR_H__

#include <glib-object.h>
#include "gspell-checker.h"

G_BEGIN_DECLS

#define GSPELL_TYPE_NAVIGATOR (gspell_navigator_get_type ())
G_DECLARE_INTERFACE (GspellNavigator, gspell_navigator,
		     GSPELL, NAVIGATOR,
		     GObject)

struct _GspellNavigatorInterface
{
	GTypeInterface parent_interface;

	gboolean	(* goto_next)		(GspellNavigator  *navigator,
						 gchar               **word,
						 GspellChecker   **spell_checker,
						 GError              **error);

	void		(* change)		(GspellNavigator *navigator,
						 const gchar         *word,
						 const gchar         *change_to);

	void		(* change_all)		(GspellNavigator *navigator,
						 const gchar         *word,
						 const gchar         *change_to);
};

gboolean	gspell_navigator_goto_next		(GspellNavigator  *navigator,
							  gchar               **word,
							  GspellChecker   **spell_checker,
							  GError              **error);

void		gspell_navigator_change		(GspellNavigator *navigator,
							  const gchar         *word,
							  const gchar         *change_to);

void		gspell_navigator_change_all	(GspellNavigator *navigator,
							  const gchar         *word,
							  const gchar         *change_to);

G_END_DECLS

#endif /* __GSPELL_NAVIGATOR_H__ */

/* ex:set ts=8 noet: */
