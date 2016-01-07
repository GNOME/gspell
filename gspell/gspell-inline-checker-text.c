/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2015, 2016 - Sébastien Wilmet
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
 * #GspellChecker to the #GtkTextBuffer. #GtkTextView:buffer changes are
 * handled, as well as #GspellChecker changes.
 */

typedef struct _GspellInlineCheckerTextPrivate GspellInlineCheckerTextPrivate;

struct _GspellInlineCheckerTextPrivate
{
	GtkTextView *view;
	GspellInlineCheckerTextBuffer *buffer_checker;
};

enum
{
	PROP_0,
	PROP_VIEW,
	PROP_ENABLED,
};

G_DEFINE_TYPE_WITH_PRIVATE (GspellInlineCheckerText, gspell_inline_checker_text, G_TYPE_OBJECT)

static void
create_buffer_checker (GspellInlineCheckerText *inline_checker)
{
	GspellInlineCheckerTextPrivate *priv;
	GtkTextBuffer *buffer;

	priv = gspell_inline_checker_text_get_instance_private (inline_checker);

	if (priv->buffer_checker != NULL)
	{
		return;
	}

	buffer = gtk_text_view_get_buffer (priv->view);
	priv->buffer_checker = _gspell_inline_checker_text_buffer_new (buffer);
	_gspell_inline_checker_text_buffer_attach_view (priv->buffer_checker,
							priv->view);
}

static void
destroy_buffer_checker (GspellInlineCheckerText *inline_checker)
{
	GspellInlineCheckerTextPrivate *priv;

	priv = gspell_inline_checker_text_get_instance_private (inline_checker);

	if (priv->view == NULL || priv->buffer_checker == NULL)
	{
		return;
	}

	_gspell_inline_checker_text_buffer_detach_view (priv->buffer_checker,
							priv->view);
	g_clear_object (&priv->buffer_checker);
}

static void
notify_buffer_cb (GtkTextView             *view,
		  GParamSpec              *pspec,
		  GspellInlineCheckerText *inline_checker)
{
	GspellInlineCheckerTextPrivate *priv;

	priv = gspell_inline_checker_text_get_instance_private (inline_checker);

	if (priv->buffer_checker == NULL)
	{
		return;
	}

	destroy_buffer_checker (inline_checker);
	create_buffer_checker (inline_checker);
}

static void
set_view (GspellInlineCheckerText *inline_checker,
	  GtkTextView             *view)
{
	GspellInlineCheckerTextPrivate *priv;

	g_return_if_fail (GTK_IS_TEXT_VIEW (view));

	priv = gspell_inline_checker_text_get_instance_private (inline_checker);

	g_assert (priv->view == NULL);
	g_assert (priv->buffer_checker == NULL);

	priv->view = view;

	g_signal_connect_object (priv->view,
				 "notify::buffer",
				 G_CALLBACK (notify_buffer_cb),
				 inline_checker,
				 0);
}

static void
gspell_inline_checker_text_get_property (GObject    *object,
					 guint       prop_id,
					 GValue     *value,
					 GParamSpec *pspec)
{
	GspellInlineCheckerText *inline_checker;
	GspellInlineCheckerTextPrivate *priv;

	inline_checker = GSPELL_INLINE_CHECKER_TEXT (object);
	priv = gspell_inline_checker_text_get_instance_private (inline_checker);

	switch (prop_id)
	{
		case PROP_VIEW:
			g_value_set_object (value, priv->view);
			break;

		case PROP_ENABLED:
			g_value_set_boolean (value, gspell_inline_checker_text_get_enabled (inline_checker));
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
	GspellInlineCheckerText *inline_checker = GSPELL_INLINE_CHECKER_TEXT (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			set_view (inline_checker, g_value_get_object (value));
			break;

		case PROP_ENABLED:
			gspell_inline_checker_text_set_enabled (inline_checker,
								g_value_get_boolean (value));
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

	if (priv->view != NULL && priv->buffer_checker != NULL)
	{
		_gspell_inline_checker_text_buffer_detach_view (priv->buffer_checker,
								priv->view);
	}

	priv->view = NULL;
	g_clear_object (&priv->buffer_checker);

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

	/**
	 * GspellInlineCheckerText:enabled:
	 *
	 * Whether the inline spell checking is enabled.
	 */
	g_object_class_install_property (object_class,
					 PROP_ENABLED,
					 g_param_spec_boolean ("enabled",
							       "Enabled",
							       "",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));
}

static void
gspell_inline_checker_text_init (GspellInlineCheckerText *inline_checker)
{
}

GspellInlineCheckerText *
gspell_inline_checker_text_new (GtkTextView *view)
{
	g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), NULL);

	return g_object_new (GSPELL_TYPE_INLINE_CHECKER_TEXT,
			     "view", view,
			     NULL);
}

/**
 * gspell_inline_checker_text_set_enabled:
 * @inline_checker: a #GspellInlineCheckerText.
 * @enabled: the new state.
 *
 * Enables or disables the inline spell checking.
 */
void
gspell_inline_checker_text_set_enabled (GspellInlineCheckerText *inline_checker,
					gboolean                 enabled)
{
	g_return_if_fail (GSPELL_IS_INLINE_CHECKER_TEXT (inline_checker));

	enabled = enabled != FALSE;

	if (enabled == gspell_inline_checker_text_get_enabled (inline_checker))
	{
		return;
	}

	if (enabled)
	{
		create_buffer_checker (inline_checker);
	}
	else
	{
		destroy_buffer_checker (inline_checker);
	}

	g_object_notify (G_OBJECT (inline_checker), "enabled");
}

/**
 * gspell_inline_checker_text_get_enabled:
 * @inline_checker: a #GspellInlineCheckerText.
 *
 * Returns: whether the inline spell checking is enabled.
 */
gboolean
gspell_inline_checker_text_get_enabled (GspellInlineCheckerText *inline_checker)
{
	GspellInlineCheckerTextPrivate *priv;

	g_return_val_if_fail (GSPELL_IS_INLINE_CHECKER_TEXT (inline_checker), FALSE);

	priv = gspell_inline_checker_text_get_instance_private (inline_checker);

	return priv->buffer_checker != NULL;
}

/* ex:set ts=8 noet: */
