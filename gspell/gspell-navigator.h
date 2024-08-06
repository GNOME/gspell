/* SPDX-FileCopyrightText: 2015, 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#if !defined (GSPELL_H_INSIDE) && !defined (GSPELL_COMPILATION)
#error "Only <gspell/gspell.h> can be included directly."
#endif

#include <glib-object.h>
#include <gspell/gspell-checker.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_NAVIGATOR (gspell_navigator_get_type ())

G_MODULE_EXPORT
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

G_MODULE_EXPORT
gboolean	gspell_navigator_goto_next	(GspellNavigator  *navigator,
						 gchar           **word,
						 GspellChecker   **spell_checker,
						 GError          **error);

G_MODULE_EXPORT
void		gspell_navigator_change		(GspellNavigator *navigator,
						 const gchar     *word,
						 const gchar     *change_to);

G_MODULE_EXPORT
void		gspell_navigator_change_all	(GspellNavigator *navigator,
						 const gchar     *word,
						 const gchar     *change_to);

G_END_DECLS
