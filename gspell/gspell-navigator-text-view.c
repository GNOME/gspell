/* SPDX-FileCopyrightText: 2015, 2016 - Sébastien Wilmet <swilmet@gnome.org>
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "gspell-config.h"
#include "gspell-navigator-text-view.h"
#include <glib/gi18n-lib.h>
#include "gspell-text-buffer.h"
#include "gspell-text-iter.h"
#include "gspell-utils.h"

/**
 * SECTION:navigator-text-view
 * @Short_description: A GspellNavigator implementation for GtkTextView
 * @Title: GspellNavigatorTextView
 * @See_also: #GspellNavigator, #GspellCheckerDialog
 *
 * #GspellNavigatorTextView is a simple implementation of the
 * #GspellNavigator interface for the #GtkTextView widget.
 *
 * If a selection exists in the #GtkTextView, only the selected text is spell
 * checked. Otherwise the whole buffer is checked.
 *
 * If only the selected text is spell checked, the implementation of
 * gspell_navigator_change_all() changes only the occurrences that were
 * present in the selection.
 *
 * The implementation of gspell_navigator_goto_next() selects the
 * misspelled word and scrolls to it.
 *
 * You need to call gspell_text_buffer_set_spell_checker() to associate a
 * #GspellChecker to the #GtkTextBuffer.
 */

typedef struct _GspellNavigatorTextViewPrivate GspellNavigatorTextViewPrivate;

struct _GspellNavigatorTextViewPrivate
{
	GtkTextView *view;
	GtkTextBuffer *buffer;

	/* Delimit the region to spell check. */
	GtkTextMark *start_boundary;
	GtkTextMark *end_boundary;

	/* Current misspelled word. */
	GtkTextMark *word_start;
	GtkTextMark *word_end;
};

enum
{
	PROP_0,
	PROP_VIEW,
};

static void gspell_navigator_iface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (GspellNavigatorTextView,
			 gspell_navigator_text_view,
			 G_TYPE_INITIALLY_UNOWNED,
			 G_ADD_PRIVATE (GspellNavigatorTextView)
			 G_IMPLEMENT_INTERFACE (GSPELL_TYPE_NAVIGATOR,
						gspell_navigator_iface_init))

static void
init_boundaries (GspellNavigatorTextView *navigator)
{
	GspellNavigatorTextViewPrivate *priv;
	GtkTextIter start;
	GtkTextIter end;

	priv = gspell_navigator_text_view_get_instance_private (navigator);

	g_return_if_fail (priv->start_boundary == NULL);
	g_return_if_fail (priv->end_boundary == NULL);

	if (!gtk_text_buffer_get_selection_bounds (priv->buffer, &start, &end))
	{
		/* No selection, take the whole buffer. */
		gtk_text_buffer_get_bounds (priv->buffer, &start, &end);
	}

	if (_gspell_text_iter_inside_word (&start) &&
	    !_gspell_text_iter_starts_word (&start))
	{
		_gspell_text_iter_backward_word_start (&start);
	}

	if (_gspell_text_iter_inside_word (&end))
	{
		_gspell_text_iter_forward_word_end (&end);
	}

	priv->start_boundary = gtk_text_buffer_create_mark (priv->buffer, NULL, &start, TRUE);
	priv->end_boundary = gtk_text_buffer_create_mark (priv->buffer, NULL, &end, FALSE);
}

static void
set_view (GspellNavigatorTextView *navigator,
	  GtkTextView             *view)
{
	GspellNavigatorTextViewPrivate *priv;

	priv = gspell_navigator_text_view_get_instance_private (navigator);

	g_return_if_fail (priv->view == NULL);
	g_return_if_fail (priv->buffer == NULL);

	priv->view = g_object_ref (view);
	priv->buffer = g_object_ref (gtk_text_view_get_buffer (view));

	init_boundaries (navigator);

	g_object_notify (G_OBJECT (navigator), "view");
}

static void
gspell_navigator_text_view_get_property (GObject    *object,
					 guint       prop_id,
					 GValue     *value,
					 GParamSpec *pspec)
{
	GspellNavigatorTextView *navigator = GSPELL_NAVIGATOR_TEXT_VIEW (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			g_value_set_object (value, gspell_navigator_text_view_get_view (navigator));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_navigator_text_view_set_property (GObject      *object,
					 guint         prop_id,
					 const GValue *value,
					 GParamSpec   *pspec)
{
	GspellNavigatorTextView *navigator = GSPELL_NAVIGATOR_TEXT_VIEW (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			set_view (navigator, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_navigator_text_view_dispose (GObject *object)
{
	GspellNavigatorTextViewPrivate *priv;

	priv = gspell_navigator_text_view_get_instance_private (GSPELL_NAVIGATOR_TEXT_VIEW (object));

	g_clear_object (&priv->view);

	if (priv->buffer != NULL)
	{
		if (priv->start_boundary != NULL)
		{
			gtk_text_buffer_delete_mark (priv->buffer, priv->start_boundary);
			priv->start_boundary = NULL;
		}

		if (priv->end_boundary != NULL)
		{
			gtk_text_buffer_delete_mark (priv->buffer, priv->end_boundary);
			priv->end_boundary = NULL;
		}

		if (priv->word_start != NULL)
		{
			gtk_text_buffer_delete_mark (priv->buffer, priv->word_start);
			priv->word_start = NULL;
		}

		if (priv->word_end != NULL)
		{
			gtk_text_buffer_delete_mark (priv->buffer, priv->word_end);
			priv->word_end = NULL;
		}

		g_object_unref (priv->buffer);
		priv->buffer = NULL;
	}

	G_OBJECT_CLASS (gspell_navigator_text_view_parent_class)->dispose (object);
}

static void
gspell_navigator_text_view_class_init (GspellNavigatorTextViewClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gspell_navigator_text_view_get_property;
	object_class->set_property = gspell_navigator_text_view_set_property;
	object_class->dispose = gspell_navigator_text_view_dispose;

	/**
	 * GspellNavigatorTextView:view:
	 *
	 * The #GtkTextView. The buffer is not sufficient, the view is needed to
	 * scroll to the misspelled words.
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
gspell_navigator_text_view_init (GspellNavigatorTextView *self)
{
}

static void
select_misspelled_word (GspellNavigatorTextView *navigator)
{
	GspellNavigatorTextViewPrivate *priv;
	GtkTextIter word_start;
	GtkTextIter word_end;

	priv = gspell_navigator_text_view_get_instance_private (navigator);

	gtk_text_buffer_get_iter_at_mark (priv->buffer, &word_start, priv->word_start);
	gtk_text_buffer_get_iter_at_mark (priv->buffer, &word_end, priv->word_end);

	gtk_text_buffer_select_range (priv->buffer, &word_start, &word_end);

	g_return_if_fail (gtk_text_view_get_buffer (priv->view) == priv->buffer);

	gtk_text_view_scroll_to_mark (priv->view,
	                              gtk_text_buffer_get_insert (priv->buffer),
	                              0.25,
	                              FALSE,
	                              0.0,
	                              0.0);
}

static gboolean
gspell_navigator_text_view_goto_next (GspellNavigator  *navigator,
				      gchar           **word_p,
				      GspellChecker   **spell_checker_p,
				      GError          **error_p)
{
	GspellNavigatorTextViewPrivate *priv;
	GspellTextBuffer *gspell_buffer;
	GspellChecker *spell_checker;
	GtkTextIter word_start;
	GtkTextIter end;
	GtkTextTag *no_spell_check_tag;

	priv = gspell_navigator_text_view_get_instance_private (GSPELL_NAVIGATOR_TEXT_VIEW (navigator));

	g_assert ((priv->word_start == NULL && priv->word_end == NULL) ||
		  (priv->word_start != NULL && priv->word_end != NULL));

	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (priv->buffer);
	spell_checker = gspell_text_buffer_get_spell_checker (gspell_buffer);

	if (spell_checker == NULL)
	{
		return FALSE;
	}

	if (gspell_checker_get_language (spell_checker) == NULL)
	{
		if (spell_checker_p != NULL)
		{
			*spell_checker_p = g_object_ref (spell_checker);
		}

		g_set_error (error_p,
			     GSPELL_CHECKER_ERROR,
			     GSPELL_CHECKER_ERROR_NO_LANGUAGE_SET,
			     "%s",
			     _("Spell checker error: no language set. "
			       "It’s maybe because no dictionaries are installed."));

		return FALSE;
	}

	gtk_text_buffer_get_iter_at_mark (priv->buffer, &end, priv->end_boundary);

	if (priv->word_start == NULL)
	{
		GtkTextIter start;

		gtk_text_buffer_get_iter_at_mark (priv->buffer, &start, priv->start_boundary);

		priv->word_start = gtk_text_buffer_create_mark (priv->buffer, NULL, &start, TRUE);
		priv->word_end = gtk_text_buffer_create_mark (priv->buffer, NULL, &start, FALSE);

		word_start = start;
	}
	else
	{
		GtkTextIter word_end;

		gtk_text_buffer_get_iter_at_mark (priv->buffer, &word_end, priv->word_end);

		if (gtk_text_iter_compare (&end, &word_end) <= 0)
		{
			return FALSE;
		}

		word_start = word_end;
	}

	no_spell_check_tag = _gspell_utils_get_no_spell_check_tag (priv->buffer);

	while (TRUE)
	{
		GtkTextIter word_end;
		gchar *word;
		gboolean correctly_spelled;
		GError *error = NULL;

		if (!_gspell_text_iter_starts_word (&word_start))
		{
			GtkTextIter iter;

			iter = word_start;
			_gspell_text_iter_forward_word_end (&word_start);

			if (gtk_text_iter_equal (&iter, &word_start))
			{
				/* Didn't move, we are at the end. */
				return FALSE;
			}

			_gspell_text_iter_backward_word_start (&word_start);
		}

		if (!_gspell_utils_skip_no_spell_check (no_spell_check_tag, &word_start, &end))
		{
			return FALSE;
		}

		g_return_val_if_fail (_gspell_text_iter_starts_word (&word_start), FALSE);

		word_end = word_start;
		_gspell_text_iter_forward_word_end (&word_end);

		if (gtk_text_iter_compare (&end, &word_end) < 0)
		{
			return FALSE;
		}

		word = gtk_text_buffer_get_text (priv->buffer, &word_start, &word_end, FALSE);

		correctly_spelled = gspell_checker_check_word (spell_checker, word, -1, &error);

		if (error != NULL)
		{
			g_propagate_error (error_p, error);
			g_free (word);
			return FALSE;
		}

		if (!correctly_spelled)
		{
			/* Found! */
			gtk_text_buffer_move_mark (priv->buffer, priv->word_start, &word_start);
			gtk_text_buffer_move_mark (priv->buffer, priv->word_end, &word_end);

			select_misspelled_word (GSPELL_NAVIGATOR_TEXT_VIEW (navigator));

			if (spell_checker_p != NULL)
			{
				*spell_checker_p = g_object_ref (spell_checker);
			}

			if (word_p != NULL)
			{
				*word_p = word;
			}
			else
			{
				g_free (word);
			}

			return TRUE;
		}

		word_start = word_end;
		g_free (word);
	}

	return FALSE;
}

static void
gspell_navigator_text_view_change (GspellNavigator *navigator,
				   const gchar     *word,
				   const gchar     *change_to)
{
	GspellNavigatorTextViewPrivate *priv;
	GtkTextIter word_start;
	GtkTextIter word_end;
	gchar *word_in_buffer = NULL;

	priv = gspell_navigator_text_view_get_instance_private (GSPELL_NAVIGATOR_TEXT_VIEW (navigator));

	g_return_if_fail (GTK_IS_TEXT_MARK (priv->word_start));
	g_return_if_fail (GTK_IS_TEXT_MARK (priv->word_end));

	gtk_text_buffer_get_iter_at_mark (priv->buffer, &word_start, priv->word_start);
	gtk_text_buffer_get_iter_at_mark (priv->buffer, &word_end, priv->word_end);

	word_in_buffer = gtk_text_buffer_get_slice (priv->buffer, &word_start, &word_end, TRUE);
	g_return_if_fail (word_in_buffer != NULL);
	g_return_if_fail (g_strcmp0 (word_in_buffer, word) == 0);
	g_free (word_in_buffer);

	gtk_text_buffer_begin_user_action (priv->buffer);

	gtk_text_buffer_delete (priv->buffer, &word_start, &word_end);
	gtk_text_buffer_insert (priv->buffer, &word_start, change_to, -1);

	gtk_text_buffer_end_user_action (priv->buffer);
}

static void
gspell_navigator_text_view_change_all (GspellNavigator *navigator,
				       const gchar     *word,
				       const gchar     *change_to)
{
	GspellNavigatorTextViewPrivate *priv;
	GtkTextIter iter;

	priv = gspell_navigator_text_view_get_instance_private (GSPELL_NAVIGATOR_TEXT_VIEW (navigator));

	g_return_if_fail (GTK_IS_TEXT_MARK (priv->start_boundary));
	g_return_if_fail (GTK_IS_TEXT_MARK (priv->end_boundary));

	gtk_text_buffer_get_iter_at_mark (priv->buffer, &iter, priv->start_boundary);

	gtk_text_buffer_begin_user_action (priv->buffer);

	while (TRUE)
	{
		gboolean found;
		GtkTextIter match_start;
		GtkTextIter match_end;
		GtkTextIter limit;

		gtk_text_buffer_get_iter_at_mark (priv->buffer, &limit, priv->end_boundary);

		found = gtk_text_iter_forward_search (&iter,
						      word,
						      GTK_TEXT_SEARCH_VISIBLE_ONLY |
						      GTK_TEXT_SEARCH_TEXT_ONLY,
						      &match_start,
						      &match_end,
						      &limit);

		if (!found)
		{
			break;
		}

		if (_gspell_text_iter_starts_word (&match_start) &&
		    _gspell_text_iter_ends_word (&match_end))
		{
			gtk_text_buffer_delete (priv->buffer, &match_start, &match_end);
			gtk_text_buffer_insert (priv->buffer, &match_end, change_to, -1);
		}

		iter = match_end;
	}

	gtk_text_buffer_end_user_action (priv->buffer);
}

static void
gspell_navigator_iface_init (gpointer g_iface,
			     gpointer iface_data)
{
	GspellNavigatorInterface *iface = g_iface;

	iface->goto_next = gspell_navigator_text_view_goto_next;
	iface->change = gspell_navigator_text_view_change;
	iface->change_all = gspell_navigator_text_view_change_all;
}

/**
 * gspell_navigator_text_view_new:
 * @view: a #GtkTextView.
 *
 * Returns: (transfer floating): a new #GspellNavigatorTextView floating object.
 */
GspellNavigator *
gspell_navigator_text_view_new (GtkTextView *view)
{
	g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), NULL);

	return g_object_new (GSPELL_TYPE_NAVIGATOR_TEXT_VIEW,
			     "view", view,
			     NULL);
}

/**
 * gspell_navigator_text_view_get_view:
 * @navigator: a #GspellNavigatorTextView.
 *
 * Returns: (transfer none): the #GtkTextView.
 */
GtkTextView *
gspell_navigator_text_view_get_view (GspellNavigatorTextView *navigator)
{
	GspellNavigatorTextViewPrivate *priv;

	g_return_val_if_fail (GSPELL_IS_NAVIGATOR_TEXT_VIEW (navigator), NULL);

	priv = gspell_navigator_text_view_get_instance_private (navigator);
	return priv->view;
}
