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

#ifndef __GSPELL_CHECKER_DIALOG_H__
#define __GSPELL_CHECKER_DIALOG_H__

#include <gtk/gtk.h>
#include "gspell-navigator.h"

G_BEGIN_DECLS

#define GSPELL_TYPE_CHECKER_DIALOG (gspell_checker_dialog_get_type ())
G_DECLARE_FINAL_TYPE (GspellCheckerDialog, gspell_checker_dialog,
		      GSPELL, CHECKER_DIALOG,
		      GtkDialog)

GtkWidget *	gspell_checker_dialog_new		(GtkWindow       *parent,
							 GspellNavigator *navigator);

G_END_DECLS

#endif  /* __GSPELL_CHECKER_DIALOG_H__ */

/* ex:set ts=8 noet: */
