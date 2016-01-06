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

#include "gspell-inline-checker-text-view.h"
#include "gspell-inline-checker-text-buffer.h"

/* Inline spell checker for GtkTextView. Handles buffer changes. */

typedef struct _GspellInlineCheckerTextViewPrivate GspellInlineCheckerTextViewPrivate;

struct _GspellInlineCheckerTextViewPrivate
{
	GtkTextView *view;
	GspellInlineCheckerTextBuffer *inline_checker;
};

enum
{
	PROP_0,
	PROP_VIEW,
};

G_DEFINE_TYPE_WITH_PRIVATE (GspellInlineCheckerTextView,
			    _gspell_inline_checker_text_view,
			    G_TYPE_OBJECT)

static void
update_inline_checker (GspellInlineCheckerTextView *self)
{
	GspellInlineCheckerTextViewPrivate *priv;
	GtkTextBuffer *buffer;

	priv = _gspell_inline_checker_text_view_get_instance_private (self);

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
notify_buffer_cb (GtkTextView                 *view,
		  GParamSpec                  *pspec,
		  GspellInlineCheckerTextView *self)
{
	update_inline_checker (self);
}

static void
set_view (GspellInlineCheckerTextView *self,
	  GtkTextView                 *view)
{
	GspellInlineCheckerTextViewPrivate *priv;

	g_return_if_fail (GTK_IS_TEXT_VIEW (view));

	priv = _gspell_inline_checker_text_view_get_instance_private (self);

	g_assert (priv->view == NULL);
	g_assert (priv->inline_checker == NULL);

	priv->view = view;

	g_signal_connect_object (priv->view,
				 "notify::buffer",
				 G_CALLBACK (notify_buffer_cb),
				 self,
				 0);

	update_inline_checker (self);
}

static void
_gspell_inline_checker_text_view_get_property (GObject    *object,
					       guint       prop_id,
					       GValue     *value,
					       GParamSpec *pspec)
{
	GspellInlineCheckerTextView *self;
	GspellInlineCheckerTextViewPrivate *priv;

	self = GSPELL_INLINE_CHECKER_TEXT_VIEW (object);
	priv = _gspell_inline_checker_text_view_get_instance_private (self);

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
_gspell_inline_checker_text_view_set_property (GObject      *object,
					       guint         prop_id,
					       const GValue *value,
					       GParamSpec   *pspec)
{
	GspellInlineCheckerTextView *self = GSPELL_INLINE_CHECKER_TEXT_VIEW (object);

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
_gspell_inline_checker_text_view_dispose (GObject *object)
{
	GspellInlineCheckerTextView *self;
	GspellInlineCheckerTextViewPrivate *priv;

	self = GSPELL_INLINE_CHECKER_TEXT_VIEW (object);
	priv = _gspell_inline_checker_text_view_get_instance_private (self);

	if (priv->view != NULL && priv->inline_checker != NULL)
	{
		_gspell_inline_checker_text_buffer_detach_view (priv->inline_checker,
								priv->view);
	}

	priv->view = NULL;
	g_clear_object (&priv->inline_checker);

	G_OBJECT_CLASS (_gspell_inline_checker_text_view_parent_class)->dispose (object);
}

static void
_gspell_inline_checker_text_view_class_init (GspellInlineCheckerTextViewClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = _gspell_inline_checker_text_view_get_property;
	object_class->set_property = _gspell_inline_checker_text_view_set_property;
	object_class->dispose = _gspell_inline_checker_text_view_dispose;

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
_gspell_inline_checker_text_view_init (GspellInlineCheckerTextView *self)
{
}

GspellInlineCheckerTextView *
_gspell_inline_checker_text_view_new (GtkTextView *view)
{
	g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), NULL);

	return g_object_new (GSPELL_TYPE_INLINE_CHECKER_TEXT_VIEW,
			     "view", view,
			     NULL);
}

/* ex:set ts=8 noet: */
