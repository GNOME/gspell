/*
 * gedit-spell-navigator.h
 * This file is part of gedit
 *
 * Copyright (C) 2015 - Sébastien Wilmet <swilmet@gnome.org>
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

#ifndef __GEDIT_SPELL_NAVIGATOR_H__
#define __GEDIT_SPELL_NAVIGATOR_H__

#include <glib-object.h>
#include "gedit-spell-checker.h"

G_BEGIN_DECLS

#define GEDIT_TYPE_SPELL_NAVIGATOR (gedit_spell_navigator_get_type ())
G_DECLARE_INTERFACE (GeditSpellNavigator, gedit_spell_navigator,
		     GEDIT, SPELL_NAVIGATOR,
		     GObject)

struct _GeditSpellNavigatorInterface
{
	GTypeInterface parent_interface;

	gboolean	(* goto_next)		(GeditSpellNavigator  *navigator,
						 gchar               **word,
						 GeditSpellChecker   **spell_checker,
						 GError              **error);

	void		(* change)		(GeditSpellNavigator *navigator,
						 const gchar         *word,
						 const gchar         *change_to);

	void		(* change_all)		(GeditSpellNavigator *navigator,
						 const gchar         *word,
						 const gchar         *change_to);
};

gboolean	gedit_spell_navigator_goto_next		(GeditSpellNavigator  *navigator,
							 gchar               **word,
							 GeditSpellChecker   **spell_checker,
							 GError              **error);

void		gedit_spell_navigator_change		(GeditSpellNavigator *navigator,
							 const gchar         *word,
							 const gchar         *change_to);

void		gedit_spell_navigator_change_all	(GeditSpellNavigator *navigator,
							 const gchar         *word,
							 const gchar         *change_to);

G_END_DECLS

#endif /* __GEDIT_SPELL_NAVIGATOR_H__ */

/* ex:set ts=8 noet: */
