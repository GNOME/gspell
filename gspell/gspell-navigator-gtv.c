/*
 * This file is part of gspell.
 *
 * Copyright 2015 - Sébastien Wilmet <swilmet@gnome.org>
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

#include "gspell-navigator-gtv.h"
#include "gspell-utils.h"

/**
 * SECTION:navigator-gtv
 * @Short_description: A GspellNavigator implementation for GtkTextView
 * @Title: GspellNavigatorGtv
 * @See_also: #GspellNavigator, #GspellCheckerDialog
 *
 * #GspellNavigatorGtv is a simple implementation of the
 * #GspellNavigator interface.
 *
 * If a selection exists in the #GtkTextView, only the selected text is spell
 * checked. Otherwise the whole buffer is checked. The same #GspellChecker
 * is used throughout the navigation.
 *
 * If only the selected text is spell checked, the implementation of
 * gspell_navigator_change_all() changes only the occurrences that were
 * present in the selection.
 *
 * The implementation of gspell_navigator_goto_next() selects the
 * misspelled word and scrolls to it.
 */

typedef struct _GspellNavigatorGtvPrivate GspellNavigatorGtvPrivate;

struct _GspellNavigatorGtvPrivate
{
	GtkTextView *view;
	GtkTextBuffer *buffer;
	GspellChecker *spell_checker;

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
	PROP_SPELL_CHECKER,
};

static void gspell_navigator_iface_init (gpointer g_iface, gpointer iface_data);

G_DEFINE_TYPE_WITH_CODE (GspellNavigatorGtv,
			 gspell_navigator_gtv,
			 G_TYPE_OBJECT,
			 G_ADD_PRIVATE (GspellNavigatorGtv)
			 G_IMPLEMENT_INTERFACE (GSPELL_TYPE_NAVIGATOR,
						gspell_navigator_iface_init))

static void
init_boundaries (GspellNavigatorGtv *navigator)
{
	GspellNavigatorGtvPrivate *priv;
	GtkTextIter start;
	GtkTextIter end;

	priv = gspell_navigator_gtv_get_instance_private (navigator);

	g_return_if_fail (priv->start_boundary == NULL);
	g_return_if_fail (priv->end_boundary == NULL);

	if (!gtk_text_buffer_get_selection_bounds (priv->buffer, &start, &end))
	{
		/* No selection, take the whole buffer. */
		gtk_text_buffer_get_bounds (priv->buffer, &start, &end);
	}

	if (gtk_text_iter_inside_word (&start) &&
	    !gtk_text_iter_starts_word (&start))
	{
		gtk_text_iter_backward_word_start (&start);
	}

	if (gtk_text_iter_inside_word (&end))
	{
		gtk_text_iter_forward_word_end (&end);
	}

	priv->start_boundary = gtk_text_buffer_create_mark (priv->buffer, NULL, &start, TRUE);
	priv->end_boundary = gtk_text_buffer_create_mark (priv->buffer, NULL, &end, FALSE);
}

static void
set_view (GspellNavigatorGtv *navigator,
	  GtkTextView        *view)
{
	GspellNavigatorGtvPrivate *priv;

	priv = gspell_navigator_gtv_get_instance_private (navigator);

	g_return_if_fail (priv->view == NULL);
	g_return_if_fail (priv->buffer == NULL);

	priv->view = g_object_ref (view);
	priv->buffer = g_object_ref (gtk_text_view_get_buffer (view));

	init_boundaries (navigator);

	g_object_notify (G_OBJECT (navigator), "view");
}

static void
set_spell_checker (GspellNavigatorGtv *navigator,
		   GspellChecker      *spell_checker)
{
	GspellNavigatorGtvPrivate *priv;

	priv = gspell_navigator_gtv_get_instance_private (navigator);

	if (g_set_object (&priv->spell_checker, spell_checker))
	{
		g_object_notify (G_OBJECT (navigator), "spell-checker");
	}
}

static void
gspell_navigator_gtv_get_property (GObject    *object,
				   guint       prop_id,
				   GValue     *value,
				   GParamSpec *pspec)
{
	GspellNavigatorGtvPrivate *priv;

	priv = gspell_navigator_gtv_get_instance_private (GSPELL_NAVIGATOR_GTV (object));

	switch (prop_id)
	{
		case PROP_VIEW:
			g_value_set_object (value, priv->view);
			break;

		case PROP_SPELL_CHECKER:
			g_value_set_object (value, priv->spell_checker);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_navigator_gtv_set_property (GObject      *object,
				   guint         prop_id,
				   const GValue *value,
				   GParamSpec   *pspec)
{
	GspellNavigatorGtv *navigator = GSPELL_NAVIGATOR_GTV (object);

	switch (prop_id)
	{
		case PROP_VIEW:
			set_view (navigator, g_value_get_object (value));
			break;

		case PROP_SPELL_CHECKER:
			set_spell_checker (navigator, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_navigator_gtv_dispose (GObject *object)
{
	GspellNavigatorGtvPrivate *priv;

	priv = gspell_navigator_gtv_get_instance_private (GSPELL_NAVIGATOR_GTV (object));

	g_clear_object (&priv->view);
	g_clear_object (&priv->spell_checker);

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

	G_OBJECT_CLASS (gspell_navigator_gtv_parent_class)->dispose (object);
}

static void
gspell_navigator_gtv_class_init (GspellNavigatorGtvClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gspell_navigator_gtv_get_property;
	object_class->set_property = gspell_navigator_gtv_set_property;
	object_class->dispose = gspell_navigator_gtv_dispose;

	/**
	 * GspellNavigatorGtv:view:
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

	/**
	 * GspellNavigatorGtv:spell-checker:
	 *
	 * The #GspellChecker to use.
	 */
	g_object_class_install_property (object_class,
					 PROP_SPELL_CHECKER,
					 g_param_spec_object ("spell-checker",
							      "Spell Checker",
							      "",
							      GSPELL_TYPE_CHECKER,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT |
							      G_PARAM_STATIC_STRINGS));
}

static void
gspell_navigator_gtv_init (GspellNavigatorGtv *self)
{
}

static void
select_misspelled_word (GspellNavigatorGtv *navigator)
{
	GspellNavigatorGtvPrivate *priv;
	GtkTextIter word_start;
	GtkTextIter word_end;

	priv = gspell_navigator_gtv_get_instance_private (navigator);

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
gspell_navigator_gtv_goto_next (GspellNavigator  *navigator,
				gchar           **word_p,
				GspellChecker   **spell_checker_p,
				GError          **error_p)
{
	GspellNavigatorGtvPrivate *priv;
	GtkTextIter word_start;
	GtkTextIter end;

	priv = gspell_navigator_gtv_get_instance_private (GSPELL_NAVIGATOR_GTV (navigator));

	g_assert ((priv->word_start == NULL && priv->word_end == NULL) ||
		  (priv->word_start != NULL && priv->word_end != NULL));

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

	while (TRUE)
	{
		GtkTextIter word_end;
		gchar *word;
		gboolean correctly_spelled;
		GError *error = NULL;

		if (!gtk_text_iter_starts_word (&word_start))
		{
			GtkTextIter iter;

			iter = word_start;
			gtk_text_iter_forward_word_end (&word_start);

			if (gtk_text_iter_equal (&iter, &word_start))
			{
				/* Didn't move, we are at the end. */
				return FALSE;
			}

			gtk_text_iter_backward_word_start (&word_start);
		}

		if (!_gspell_utils_skip_no_spell_check (&word_start, &end))
		{
			return FALSE;
		}

		g_return_val_if_fail (gtk_text_iter_starts_word (&word_start), FALSE);

		word_end = word_start;
		gtk_text_iter_forward_word_end (&word_end);

		if (gtk_text_iter_compare (&end, &word_end) < 0)
		{
			return FALSE;
		}

		word = gtk_text_buffer_get_text (priv->buffer, &word_start, &word_end, FALSE);

		correctly_spelled = gspell_checker_check_word (priv->spell_checker, word, &error);

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

			select_misspelled_word (GSPELL_NAVIGATOR_GTV (navigator));

			if (spell_checker_p != NULL)
			{
				*spell_checker_p = g_object_ref (priv->spell_checker);
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
gspell_navigator_gtv_change (GspellNavigator *navigator,
			     const gchar     *word,
			     const gchar     *change_to)
{
	GspellNavigatorGtvPrivate *priv;
	GtkTextIter word_start;
	GtkTextIter word_end;
	gchar *word_in_buffer = NULL;

	priv = gspell_navigator_gtv_get_instance_private (GSPELL_NAVIGATOR_GTV (navigator));

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
gspell_navigator_gtv_change_all (GspellNavigator *navigator,
				 const gchar     *word,
				 const gchar     *change_to)
{
	GspellNavigatorGtvPrivate *priv;
	GtkTextIter iter;

	priv = gspell_navigator_gtv_get_instance_private (GSPELL_NAVIGATOR_GTV (navigator));

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

		if (gtk_text_iter_starts_word (&match_start) &&
		    gtk_text_iter_ends_word (&match_end))
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

	iface->goto_next = gspell_navigator_gtv_goto_next;
	iface->change = gspell_navigator_gtv_change;
	iface->change_all = gspell_navigator_gtv_change_all;
}

/**
 * gspell_navigator_gtv_new:
 * @view: a #GtkTextView.
 * @spell_checker: a #GspellChecker.
 *
 * Returns: (transfer full): a new #GspellNavigatorGtv object.
 */
GspellNavigator *
gspell_navigator_gtv_new (GtkTextView   *view,
			  GspellChecker *spell_checker)
{
	g_return_val_if_fail (GTK_IS_TEXT_VIEW (view), NULL);
	g_return_val_if_fail (GSPELL_IS_CHECKER (spell_checker), NULL);

	return g_object_new (GSPELL_TYPE_NAVIGATOR_GTV,
			     "view", view,
			     "spell-checker", spell_checker,
			     NULL);
}

/* ex:set ts=8 noet: */
