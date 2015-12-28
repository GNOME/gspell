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

#ifndef __GSPELL_BUFFER_NOTIFIER_H__
#define __GSPELL_BUFFER_NOTIFIER_H__

#include <gtk/gtk.h>
#include "gspell-checker.h"

G_BEGIN_DECLS

#define GSPELL_TYPE_BUFFER_NOTIFIER (_gspell_buffer_notifier_get_type ())

G_GNUC_INTERNAL
G_DECLARE_FINAL_TYPE (GspellBufferNotifier, _gspell_buffer_notifier,
		      GSPELL, BUFFER_NOTIFIER,
		      GObject)

G_GNUC_INTERNAL
GspellBufferNotifier *	_gspell_buffer_notifier_get_instance			(void);

G_GNUC_INTERNAL
void			_gspell_buffer_notifier_text_buffer_checker_changed	(GspellBufferNotifier *notifier,
										 GtkTextBuffer        *buffer,
										 GspellChecker        *new_checker);

G_END_DECLS

#endif /* __GSPELL_BUFFER_NOTIFIER_H__ */

/* ex:set ts=8 noet: */
