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

#ifndef __GSPELL_AUTOMATIC_SPELL_CHECKER_H__
#define __GSPELL_AUTOMATIC_SPELL_CHECKER_H__

#include <gtksourceview/gtksource.h>
#include "gspell-spell-checker.h"

#define GSPELL_TYPE_AUTOMATIC_SPELL_CHECKER (gspell_automatic_spell_checker_get_type ())
G_DECLARE_FINAL_TYPE (GspellAutomaticSpellChecker, gspell_automatic_spell_checker,
		      GSPELL, AUTOMATIC_SPELL_CHECKER,
		      GObject)

GspellAutomaticSpellChecker *
	gspell_automatic_spell_checker_new		(GtkSourceBuffer   *buffer,
							  GspellSpellChecker *checker);

void	gspell_automatic_spell_checker_attach_view	(GspellAutomaticSpellChecker *spell,
							  GtkTextView                *view);

void	gspell_automatic_spell_checker_detach_view	(GspellAutomaticSpellChecker *spell,
							  GtkTextView                *view);

#endif  /* __GSPELL_AUTOMATIC_SPELL_CHECKER_H__ */

/* ex:set ts=8 noet: */
