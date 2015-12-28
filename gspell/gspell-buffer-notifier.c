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

#include "gspell-buffer-notifier.h"

struct _GspellBufferNotifier
{
	GObject parent;
};

enum
{
	SIGNAL_TEXT_BUFFER_CHECKER_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (GspellBufferNotifier, _gspell_buffer_notifier, G_TYPE_OBJECT)

static void
_gspell_buffer_notifier_class_init (GspellBufferNotifierClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	/**
	 * GspellBufferNotifier::text-buffer-checker-changed:
	 * @notifier: the #GspellBufferNotifier singleton.
	 * @buffer: the #GtkTextBuffer.
	 * @new_checker: (nullable): the new #GspellChecker, or %NULL.
	 *
	 * Emitted when a different spell checker is associated to @buffer.
	 */
	signals[SIGNAL_TEXT_BUFFER_CHECKER_CHANGED] =
		g_signal_new ("text-buffer-checker-changed",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      0,
			      NULL, NULL, NULL,
			      G_TYPE_NONE,
			      2,
			      GTK_TYPE_TEXT_BUFFER,
			      GSPELL_TYPE_CHECKER);
}

static void
_gspell_buffer_notifier_init (GspellBufferNotifier *notifier)
{
}

GspellBufferNotifier *
_gspell_buffer_notifier_get_instance (void)
{
	static GspellBufferNotifier *instance = NULL;

	if (instance == NULL)
	{
		instance = g_object_new (GSPELL_TYPE_BUFFER_NOTIFIER, NULL);
	}

	return instance;
}

void
_gspell_buffer_notifier_text_buffer_checker_changed (GspellBufferNotifier *notifier,
						     GtkTextBuffer        *buffer,
						     GspellChecker        *new_checker)
{
	g_return_if_fail (GSPELL_IS_BUFFER_NOTIFIER (notifier));
	g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
	g_return_if_fail (new_checker == NULL || GSPELL_IS_CHECKER (new_checker));

	g_signal_emit (notifier,
		       signals[SIGNAL_TEXT_BUFFER_CHECKER_CHANGED], 0,
		       buffer,
		       new_checker);
}

/* ex:set ts=8 noet: */
