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

#include "gspell-text-view.h"
#include "gspell-inline-checker-text-buffer.h"

/**
 * SECTION:text-view
 * @Title: GspellTextView
 * @Short_description: Spell checking support for GtkTextView
 *
 * #GspellTextView extends the #GtkTextView class with inline spell checking.
 * Misspelled words are highlighted with a %PANGO_UNDERLINE_ERROR, usually a red
 * wavy underline. Right-clicking a misspelled word pops up a context menu of
 * suggested replacements. The context menu also contains an “Ignore All” item
 * to add the misspelled word to the session dictionary. And an “Add” item to
 * add the word to the personal dictionary.
 *
 * The spell is checked only on the visible region of the #GtkTextView. Note
 * that if a same #GtkTextBuffer is used for several views, the misspelled words
 * are visible in all views, because the highlighting is achieved with a
 * #GtkTextTag added to the buffer.
 *
 * You need to call gspell_text_buffer_set_spell_checker() to associate a
 * #GspellChecker to the #GtkTextBuffer. #GspellTextView handles automatically
 * changes to the following properties: #GtkTextView:buffer,
 * #GspellTextBuffer:spell-checker and #GspellChecker:language.
 *
 * Note that #GspellTextView extends the #GtkTextView class but without
 * subclassing it, because the GtkSourceView library has already a #GtkTextView
 * subclass.
 */

typedef struct _GspellTextViewPrivate GspellTextViewPrivate;

struct _GspellTextViewPrivate
{
	GtkTextView *view;
	GspellInlineCheckerTextBuffer *inline_checker;
};

enum
{
	PROP_0,
	PROP_VIEW,
	PROP_INLINE_SPELL_CHECKING,
};

#define GSPELL_TEXT_VIEW_KEY "gspell-text-view-key"

G_DEFINE_TYPE_WITH_PRIVATE (GspellTextView, gspell_text_view, G_TYPE_OBJECT)

static void
create_inline_checker (GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;
	GtkTextBuffer *buffer;

	priv = gspell_text_view_get_instance_private (gspell_view);

	if (priv->inline_checker != NULL)
	{
		return;
	}

	buffer = gtk_text_view_get_buffer (priv->view);
	priv->inline_checker = _gspell_inline_checker_text_buffer_new (buffer);
	_gspell_inline_checker_text_buffer_attach_view (priv->inline_checker,
							priv->view);
}

static void
destroy_inline_checker (GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	priv = gspell_text_view_get_instance_private (gspell_view);

	if (priv->view == NULL || priv->inline_checker == NULL)
	{
		return;
	}

	_gspell_inline_checker_text_buffer_detach_view (priv->inline_checker,
							priv->view);
	g_clear_object (&priv->inline_checker);
}

static void
notify_buffer_cb (GtkTextView    *gtk_view,
		  GParamSpec     *pspec,
		  GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	priv = gspell_text_view_get_instance_private (gspell_view);

	if (priv->inline_checker == NULL)
	{
		return;
	}

	destroy_inline_checker (gspell_view);
	create_inline_checker (gspell_view);
}

static void
set_view (GspellTextView *gspell_view,
	  GtkTextView    *gtk_view)
{
	GspellTextViewPrivate *priv;

	g_return_if_fail (GTK_IS_TEXT_VIEW (gtk_view));

	priv = gspell_text_view_get_instance_private (gspell_view);

	g_assert (priv->view == NULL);
	g_assert (priv->inline_checker == NULL);

	priv->view = gtk_view;

	g_signal_connect_object (priv->view,
				 "notify::buffer",
				 G_CALLBACK (notify_buffer_cb),
				 gspell_view,
				 0);

	g_object_notify (G_OBJECT (gspell_view), "view");
}

static void
gspell_text_view_get_property (GObject    *object,
			       guint       prop_id,
			       GValue     *value,
			       GParamSpec *pspec)
{
	GspellTextView *gspell_view = GSPELL_TEXT_VIEW (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			g_value_set_object (value, gspell_text_view_get_view (gspell_view));
			break;

		case PROP_INLINE_SPELL_CHECKING:
			g_value_set_boolean (value, gspell_text_view_get_inline_spell_checking (gspell_view));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_text_view_set_property (GObject      *object,
			       guint         prop_id,
			       const GValue *value,
			       GParamSpec   *pspec)
{
	GspellTextView *gspell_view = GSPELL_TEXT_VIEW (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			set_view (gspell_view, g_value_get_object (value));
			break;

		case PROP_INLINE_SPELL_CHECKING:
			gspell_text_view_set_inline_spell_checking (gspell_view, g_value_get_boolean (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_text_view_dispose (GObject *object)
{
	GspellTextViewPrivate *priv;

	priv = gspell_text_view_get_instance_private (GSPELL_TEXT_VIEW (object));

	if (priv->view != NULL && priv->inline_checker != NULL)
	{
		_gspell_inline_checker_text_buffer_detach_view (priv->inline_checker,
								priv->view);
	}

	priv->view = NULL;
	g_clear_object (&priv->inline_checker);

	G_OBJECT_CLASS (gspell_text_view_parent_class)->dispose (object);
}

static void
gspell_text_view_class_init (GspellTextViewClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gspell_text_view_get_property;
	object_class->set_property = gspell_text_view_set_property;
	object_class->dispose = gspell_text_view_dispose;

	/**
	 * GspellTextView:view:
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
	 * GspellTextView:inline-spell-checking:
	 *
	 * Whether the inline spell checking is enabled.
	 */
	g_object_class_install_property (object_class,
					 PROP_INLINE_SPELL_CHECKING,
					 g_param_spec_boolean ("inline-spell-checking",
							       "Inline Spell Checking",
							       "",
							       FALSE,
							       G_PARAM_READWRITE |
							       G_PARAM_STATIC_STRINGS));
}

static void
gspell_text_view_init (GspellTextView *gspell_view)
{
}

/**
 * gspell_text_view_get_from_gtk_text_view:
 * @gtk_view: a #GtkTextView.
 *
 * Returns the #GspellTextView of @gtk_view. The returned object is guaranteed
 * to be the same for the lifetime of @gtk_view.
 *
 * Returns: (transfer none): the #GspellTextView of @gtk_view.
 */
GspellTextView *
gspell_text_view_get_from_gtk_text_view (GtkTextView *gtk_view)
{
	GspellTextView *gspell_view;

	g_return_val_if_fail (GTK_IS_TEXT_VIEW (gtk_view), NULL);

	gspell_view = g_object_get_data (G_OBJECT (gtk_view), GSPELL_TEXT_VIEW_KEY);

	if (gspell_view == NULL)
	{
		gspell_view = g_object_new (GSPELL_TYPE_TEXT_VIEW,
					    "view", gtk_view,
					    NULL);

		g_object_set_data_full (G_OBJECT (gtk_view),
					GSPELL_TEXT_VIEW_KEY,
					gspell_view,
					g_object_unref);
	}

	g_return_val_if_fail (GSPELL_IS_TEXT_VIEW (gspell_view), NULL);
	return gspell_view;
}

/**
 * gspell_text_view_get_view:
 * @gspell_view: a #GspellTextView.
 *
 * Returns: (transfer none): the #GtkTextView of @gspell_view.
 */
GtkTextView *
gspell_text_view_get_view (GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	g_return_val_if_fail (GSPELL_IS_TEXT_VIEW (gspell_view), NULL);

	priv = gspell_text_view_get_instance_private (gspell_view);
	return priv->view;
}

/**
 * gspell_text_view_get_inline_spell_checking:
 * @gspell_view: a #GspellTextView.
 *
 * Returns: whether the inline spell checking is enabled.
 */
gboolean
gspell_text_view_get_inline_spell_checking (GspellTextView *gspell_view)
{
	GspellTextViewPrivate *priv;

	g_return_val_if_fail (GSPELL_IS_TEXT_VIEW (gspell_view), FALSE);

	priv = gspell_text_view_get_instance_private (gspell_view);
	return priv->inline_checker != NULL;
}

/**
 * gspell_text_view_set_inline_spell_checking:
 * @gspell_view: a #GspellTextView.
 * @enable: the new state.
 *
 * Enables or disables the inline spell checking.
 */
void
gspell_text_view_set_inline_spell_checking (GspellTextView *gspell_view,
					    gboolean        enable)
{
	g_return_if_fail (GSPELL_IS_TEXT_VIEW (gspell_view));

	enable = enable != FALSE;

	if (enable == gspell_text_view_get_inline_spell_checking (gspell_view))
	{
		return;
	}

	if (enable)
	{
		create_inline_checker (gspell_view);
	}
	else
	{
		destroy_inline_checker (gspell_view);
	}

	g_object_notify (G_OBJECT (gspell_view), "inline-spell-checking");
}

/* ex:set ts=8 noet: */
