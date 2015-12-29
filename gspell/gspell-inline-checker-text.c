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

#include "gspell-inline-checker-text.h"
#include "gspell-inline-checker-text-buffer.h"

/**
 * SECTION:inline-checker-text
 * @Short_description: Inline spell checker for GtkTextView
 * @Title: GspellInlineCheckerText
 * @See_also: #GspellChecker
 *
 * The #GspellInlineCheckerText is an inline spell checker for the
 * #GtkTextView widget. Misspelled words are highlighted with a
 * %PANGO_UNDERLINE_ERROR, usually a red wavy underline. Right-clicking a
 * misspelled word pops up a context menu of suggested replacements. The context
 * menu also contains an “Ignore All” item to add the misspelled word to the
 * session dictionary. And an “Add” item to add the word to the personal
 * dictionary.
 *
 * The spell is checked only on the visible region of the #GtkTextView. Note
 * that if a same #GtkTextBuffer is used for several views, the misspelled words
 * are visible in all views, because the highlighting is achieved with a
 * #GtkTextTag added to the buffer.
 *
 * You need to call gspell_text_buffer_set_spell_checker() to associate a
 * #GspellChecker to the #GtkTextBuffer. You can call
 * gspell_text_buffer_set_spell_checker() at any time, a
 * #GspellInlineCheckerText re-checks the buffer when the #GspellChecker
 * changes.
 *
 * #GspellInlineCheckerText supports buffer changes.
 */

typedef struct _GspellInlineCheckerTextPrivate GspellInlineCheckerTextPrivate;

struct _GspellInlineCheckerTextPrivate
{
	GtkTextView *view;
	GspellInlineCheckerTextBuffer *inline_checker;
};

enum
{
	PROP_0,
	PROP_VIEW,
};

G_DEFINE_TYPE_WITH_PRIVATE (GspellInlineCheckerText, gspell_inline_checker_text, G_TYPE_OBJECT)

static void
update_inline_checker (GspellInlineCheckerText *self)
{
	GspellInlineCheckerTextPrivate *priv;
	GtkTextBuffer *buffer;

	priv = gspell_inline_checker_text_get_instance_private (self);

	if (priv->view == NULL)
	{
		return;
	}

	if (priv->inline_checker != NULL)
	{
		_gspell_inline_checker_text_buffer_detach_view (priv->inline_checker,
								priv->view);
		g_object_unref (priv->inline_checker);
	}

	buffer = gtk_text_view_get_buffer (priv->view);
	priv->inline_checker = _gspell_inline_checker_text_buffer_new (buffer);
	_gspell_inline_checker_text_buffer_attach_view (priv->inline_checker,
							priv->view);
}

static void
notify_buffer_cb (GtkTextView             *view,
		  GParamSpec              *pspec,
		  GspellInlineCheckerText *self)
{
	update_inline_checker (self);
}

static void
set_view (GspellInlineCheckerText *self,
	  GtkTextView             *view)
{
	GspellInlineCheckerTextPrivate *priv;

	g_return_if_fail (GTK_IS_TEXT_VIEW (view));

	priv = gspell_inline_checker_text_get_instance_private (self);

	g_assert (priv->view == NULL);
	g_assert (priv->inline_checker == NULL);

	priv->view = g_object_ref (view);

	g_signal_connect_object (priv->view,
				 "notify::buffer",
				 G_CALLBACK (notify_buffer_cb),
				 self,
				 0);

	update_inline_checker (self);
}

static void
gspell_inline_checker_text_get_property (GObject    *object,
					 guint       prop_id,
					 GValue     *value,
					 GParamSpec *pspec)
{
	GspellInlineCheckerTextPrivate *priv;

	priv = gspell_inline_checker_text_get_instance_private (GSPELL_INLINE_CHECKER_TEXT (object));

	switch (prop_id)
	{
		case PROP_VIEW:
			g_value_set_object (value, priv->view);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_inline_checker_text_set_property (GObject      *object,
					 guint         prop_id,
					 const GValue *value,
					 GParamSpec   *pspec)
{
	GspellInlineCheckerText *self = GSPELL_INLINE_CHECKER_TEXT (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			set_view (self, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_inline_checker_text_dispose (GObject *object)
{
	GspellInlineCheckerTextPrivate *priv;

	priv = gspell_inline_checker_text_get_instance_private (GSPELL_INLINE_CHECKER_TEXT (object));

	if (priv->view != NULL && priv->inline_checker != NULL)
	{
		_gspell_inline_checker_text_buffer_detach_view (priv->inline_checker,
								priv->view);
	}

	g_clear_object (&priv->view);
	g_clear_object (&priv->inline_checker);

	G_OBJECT_CLASS (gspell_inline_checker_text_parent_class)->dispose (object);
}

static void
gspell_inline_checker_text_class_init (GspellInlineCheckerTextClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gspell_inline_checker_text_get_property;
	object_class->set_property = gspell_inline_checker_text_set_property;
	object_class->dispose = gspell_inline_checker_text_dispose;

	/**
	 * GspellInlineCheckerText:view:
	 *
	 * The #GtkTextView.
	 */
	g_object_class_install_property (object_class,
					 PROP_VIEW,
					 g_param_spec_object ("view",
							      "View",
							      "",
							      GTK_TYPE_TEXT_VIEW,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));
}

static void
gspell_inline_checker_text_init (GspellInlineCheckerText *self)
{
}

/**
 * gspell_inline_checker_text_new:
 * @view: a #GtkTextView.
 *
 * Returns: a new #GspellInlineCheckerText object.
 */
GspellInlineCheckerText *
gspell_inline_checker_text_new (GtkTextView *view)
{
	g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), NULL);

	return g_object_new (GSPELL_TYPE_INLINE_CHECKER_TEXT,
			     "view", view,
			     NULL);
}

/* ex:set ts=8 noet: */
