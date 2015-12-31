/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2015 - Sébastien Wilmet
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

#include "gspell-text-view.h"
#include "gspell-inline-checker-text.h"

#define INLINE_CHECKER_KEY "gspell-text-view-inline-checker-key"

void
gspell_text_view_set_inline_checking (GtkTextView *view,
				      gboolean     enable)
{
	g_return_if_fail (GTK_IS_TEXT_VIEW (view));

	if (enable)
	{
		if (gspell_text_view_get_inline_checking (view))
		{
			return;
		}

		g_object_set_data_full (G_OBJECT (view),
					INLINE_CHECKER_KEY,
					gspell_inline_checker_text_new (view),
					g_object_unref);
	}
	else
	{
		g_object_set_data (G_OBJECT (view),
				   INLINE_CHECKER_KEY,
				   NULL);
	}
}

gboolean
gspell_text_view_get_inline_checking (GtkTextView *view)
{
	g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), FALSE);

	return g_object_get_data (G_OBJECT (view), INLINE_CHECKER_KEY) != NULL;
}

/* ex:set ts=8 noet: */
