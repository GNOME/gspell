/*
 * This file is part of gspell.
 *
 * Copyright 2002 - Paolo Maggi
 * Copyright 2015 - Sébastien Wilmet
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

/* This is a modified version of GtkSpell 2.0.2 (gtkspell.sf.net)
 * Copyright 2002 - Evan Martin
 */

#ifndef __GEDIT_AUTOMATIC_SPELL_CHECKER_H__
#define __GEDIT_AUTOMATIC_SPELL_CHECKER_H__

#include <gtksourceview/gtksource.h>
#include "gedit-spell-checker.h"

#define GEDIT_TYPE_AUTOMATIC_SPELL_CHECKER (gedit_automatic_spell_checker_get_type ())
G_DECLARE_FINAL_TYPE (GeditAutomaticSpellChecker, gedit_automatic_spell_checker,
		      GEDIT, AUTOMATIC_SPELL_CHECKER,
		      GObject)

GeditAutomaticSpellChecker *
	gedit_automatic_spell_checker_new		(GtkSourceBuffer   *buffer,
							 GeditSpellChecker *checker);

void	gedit_automatic_spell_checker_attach_view	(GeditAutomaticSpellChecker *spell,
							 GtkTextView                *view);

void	gedit_automatic_spell_checker_detach_view	(GeditAutomaticSpellChecker *spell,
							 GtkTextView                *view);

#endif  /* __GEDIT_AUTOMATIC_SPELL_CHECKER_H__ */

/* ex:set ts=8 noet: */
