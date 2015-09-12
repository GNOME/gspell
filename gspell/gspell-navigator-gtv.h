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

#ifndef __GSPELL_SPELL_NAVIGATOR_GTV_H__
#define __GSPELL_SPELL_NAVIGATOR_GTV_H__

#include <gtk/gtk.h>
#include "gspell-navigator.h"
#include "gspell-checker.h"

G_BEGIN_DECLS

#define GSPELL_TYPE_SPELL_NAVIGATOR_GTV (gspell_spell_navigator_gtv_get_type ())
G_DECLARE_DERIVABLE_TYPE (GspellSpellNavigatorGtv, gspell_spell_navigator_gtv,
			  GSPELL, SPELL_NAVIGATOR_GTV,
			  GObject)

struct _GspellSpellNavigatorGtvClass
{
	GObjectClass parent_class;
};

GspellSpellNavigator *	gspell_spell_navigator_gtv_new		(GtkTextView       *view,
								  GspellSpellChecker *spell_checker);

G_END_DECLS

#endif /* __GSPELL_SPELL_NAVIGATOR_GTV_H__ */

/* ex:set ts=8 noet: */
