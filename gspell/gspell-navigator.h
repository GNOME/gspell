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

#ifndef __GSPELL_SPELL_NAVIGATOR_H__
#define __GSPELL_SPELL_NAVIGATOR_H__

#include <glib-object.h>
#include "gspell-checker.h"

G_BEGIN_DECLS

#define GSPELL_TYPE_SPELL_NAVIGATOR (gspell_spell_navigator_get_type ())
G_DECLARE_INTERFACE (GspellSpellNavigator, gspell_spell_navigator,
		     GSPELL, SPELL_NAVIGATOR,
		     GObject)

struct _GspellSpellNavigatorInterface
{
	GTypeInterface parent_interface;

	gboolean	(* goto_next)		(GspellSpellNavigator  *navigator,
						 gchar               **word,
						 GspellSpellChecker   **spell_checker,
						 GError              **error);

	void		(* change)		(GspellSpellNavigator *navigator,
						 const gchar         *word,
						 const gchar         *change_to);

	void		(* change_all)		(GspellSpellNavigator *navigator,
						 const gchar         *word,
						 const gchar         *change_to);
};

gboolean	gspell_spell_navigator_goto_next		(GspellSpellNavigator  *navigator,
							  gchar               **word,
							  GspellSpellChecker   **spell_checker,
							  GError              **error);

void		gspell_spell_navigator_change		(GspellSpellNavigator *navigator,
							  const gchar         *word,
							  const gchar         *change_to);

void		gspell_spell_navigator_change_all	(GspellSpellNavigator *navigator,
							  const gchar         *word,
							  const gchar         *change_to);

G_END_DECLS

#endif /* __GSPELL_SPELL_NAVIGATOR_H__ */

/* ex:set ts=8 noet: */
