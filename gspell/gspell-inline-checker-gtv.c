/*
 * This file is part of gspell.
 *
 * Copyright 2002 - Paolo Maggi
 * Copyright 2015 - Sébastien Wilmet
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

/* This is a modified version of GtkSpell 2.0.5 (gtkspell.sf.net)
 * Copyright 2002 - Evan Martin
 */

#include "config.h"
#include "gspell-inline-checker-gtv.h"
#include <string.h>
#include <glib/gi18n-lib.h>
#include "gspell-utils.h"
#include "gtktextregion.h"

/**
 * SECTION:inline-checker-gtv
 * @Short_description: Inline spell checker for GtkTextView
 * @Title: GspellInlineCheckerGtv
 * @See_also: #GspellChecker
 *
 * The #GspellInlineCheckerGtv is an inline spell checker for the
 * #GtkTextView widget. Misspelled words are highlighted with a
 * %PANGO_UNDERLINE_ERROR, usually a red wavy underline. Right-clicking a
 * misspelled word pops up a context menu of suggested replacements. The context
 * menu also contains an “Ignore All” item to add the misspelled word to the
 * session dictionary. And an “Add” item to add the word to the personal
 * dictionary.
 *
 * The spell is checked only on the visible regions of the attached
 * #GtkTextView's.
 */

struct _GspellInlineCheckerGtv
{
	GObject parent;

	GtkTextBuffer *buffer;
	GspellChecker *spell_checker;

	/* List of GtkTextView* */
	GSList *views;

	GtkTextTag *tag_highlight;

	GtkTextMark *mark_click;

	GtkTextRegion *scan_region;
	guint timeout_id;
};

enum
{
	PROP_0,
	PROP_BUFFER,
	PROP_SPELL_CHECKER,
};

#define ENABLE_DEBUG 0

#define INLINE_CHECKER_GTV_KEY	"GspellInlineCheckerGtvID"
#define SUGGESTION_KEY		"GspellInlineSuggestionID"

/* Timeout durations in milliseconds. Writing and deleting text should be smooth
 * and responsive.
 */
#define TIMEOUT_DURATION_BUFFER_MODIFIED 400
#define TIMEOUT_DURATION_DRAWING 20

G_DEFINE_TYPE (GspellInlineCheckerGtv, gspell_inline_checker_gtv, G_TYPE_OBJECT)

static void
check_word (GspellInlineCheckerGtv *spell,
	    const GtkTextIter      *start,
	    const GtkTextIter      *end)
{
	gchar *word;
	GError *error = NULL;
	gboolean correctly_spelled;

	if (!gtk_text_iter_starts_word (start) ||
	    !gtk_text_iter_ends_word (end))
	{
		g_warning ("Spell checking: @start and @end must delimit a word");
		return;
	}

	word = gtk_text_buffer_get_text (spell->buffer, start, end, FALSE);

	correctly_spelled = gspell_checker_check_word (spell->spell_checker,
						       word,
						       &error);

	if (error != NULL)
	{
		g_warning ("Automatic spell checker: %s", error->message);
		g_error_free (error);
	}

	if (!correctly_spelled)
	{
		gtk_text_buffer_apply_tag (spell->buffer,
					   spell->tag_highlight,
					   start,
					   end);
	}

	g_free (word);
}

static void
adjust_iters_at_word_boundaries (GtkTextIter *start,
				 GtkTextIter *end)
{
	if (gtk_text_iter_inside_word (end))
	{
		gtk_text_iter_forward_word_end (end);
	}

	if (!gtk_text_iter_starts_word (start))
	{
		if (gtk_text_iter_inside_word (start) ||
		    gtk_text_iter_ends_word (start))
		{
			gtk_text_iter_backward_word_start (start);
		}
		else
		{
			/* If we're neither at the beginning nor inside a word,
			 * me must be in some spaces.
			 * Skip forward to the beginning of the next word.
			 */
			if (gtk_text_iter_forward_word_end (start) ||
			    gtk_text_iter_is_end (start))
			{
				gtk_text_iter_backward_word_start (start);
			}
		}
	}
}

static void
check_subregion (GspellInlineCheckerGtv *spell,
		 const GtkTextIter      *start,
		 const GtkTextIter      *end)
{
	GtkTextIter start_adjusted;
	GtkTextIter end_adjusted;
	GtkTextIter word_start;

	start_adjusted = *start;
	end_adjusted = *end;
	adjust_iters_at_word_boundaries (&start_adjusted, &end_adjusted);

	gtk_text_buffer_remove_tag (spell->buffer,
				    spell->tag_highlight,
				    &start_adjusted,
				    &end_adjusted);

	word_start = start_adjusted;

	while (_gspell_utils_skip_no_spell_check (&word_start, &end_adjusted) &&
	       gtk_text_iter_compare (&word_start, &end_adjusted) < 0)
	{
		GtkTextIter word_end;
		GtkTextIter next_word_start;

		word_end = word_start;
		gtk_text_iter_forward_word_end (&word_end);

		if (gtk_text_iter_starts_word (&word_start) &&
		    gtk_text_iter_ends_word (&word_end))
		{
			check_word (spell, &word_start, &word_end);
		}

		next_word_start = word_end;
		gtk_text_iter_forward_word_end (&next_word_start);
		gtk_text_iter_backward_word_start (&next_word_start);

		/* Make sure we've actually advanced (we don't advance if we are
		 * at the end of the buffer).
		 */
		if (gtk_text_iter_compare (&next_word_start, &word_start) <= 0)
		{
			break;
		}

		word_start = next_word_start;
	}
}

static void
check_region (GspellInlineCheckerGtv *spell,
	      GtkTextRegion          *region)
{
	GtkTextRegionIterator region_iter;

	if (region == NULL)
	{
		return;
	}

	gtk_text_region_get_iterator (region, &region_iter, 0);

	while (!gtk_text_region_iterator_is_end (&region_iter))
	{
		GtkTextIter start;
		GtkTextIter end;

		if (!gtk_text_region_iterator_get_subregion (&region_iter, &start, &end))
		{
			return;
		}

		check_subregion (spell, &start, &end);

		gtk_text_region_iterator_next (&region_iter);
	}
}

static void
get_visible_region (GtkTextView *view,
		    GtkTextIter *start,
		    GtkTextIter *end)
{
	GdkRectangle visible_rect;

	gtk_text_view_get_visible_rect (view, &visible_rect);

	gtk_text_view_get_line_at_y (view,
				     start,
				     visible_rect.y,
				     NULL);

	gtk_text_view_get_line_at_y (view,
				     end,
				     visible_rect.y + visible_rect.height,
				     NULL);

	gtk_text_iter_backward_line (start);
	gtk_text_iter_forward_line (end);
}

/* A TextRegion can contain empty subregions. So checking the number of
 * subregions is not sufficient.
 * When calling gtk_text_region_add() with equal iters, the subregion is not
 * added. But when a subregion becomes empty, due to text deletion, the
 * subregion is not removed from the TextRegion.
 */
static gboolean
is_text_region_empty (GtkTextRegion *region)
{
	GtkTextRegionIterator region_iter;

	if (region == NULL)
	{
		return TRUE;
	}

	gtk_text_region_get_iterator (region, &region_iter, 0);

	while (!gtk_text_region_iterator_is_end (&region_iter))
	{
		GtkTextIter region_start;
		GtkTextIter region_end;

		if (!gtk_text_region_iterator_get_subregion (&region_iter,
							     &region_start,
							     &region_end))
		{
			return TRUE;
		}

		if (!gtk_text_iter_equal (&region_start, &region_end))
		{
			return FALSE;
		}

		gtk_text_region_iterator_next (&region_iter);
	}

	return TRUE;
}

static void
check_visible_region_in_view (GspellInlineCheckerGtv *spell,
			      GtkTextView            *view)
{
	GtkTextIter visible_start;
	GtkTextIter visible_end;
	GtkTextRegion *intersect;

	if (spell->scan_region == NULL)
	{
		return;
	}

	get_visible_region (view, &visible_start, &visible_end);

	intersect = gtk_text_region_intersect (spell->scan_region,
					       &visible_start,
					       &visible_end);

	if (intersect == NULL)
	{
		return;
	}

	check_region (spell, intersect);
	gtk_text_region_destroy (intersect);

	gtk_text_region_subtract (spell->scan_region,
				  &visible_start,
				  &visible_end);

	if (is_text_region_empty (spell->scan_region))
	{
		gtk_text_region_destroy (spell->scan_region);
		spell->scan_region = NULL;
	}
}

static void
check_visible_region (GspellInlineCheckerGtv *spell)
{
	GSList *l;
#if ENABLE_DEBUG
	GTimer *timer;
#endif

	if (spell->scan_region == NULL)
	{
		return;
	}

#if ENABLE_DEBUG
	timer = g_timer_new ();
#endif

	for (l = spell->views; l != NULL; l = l->next)
	{
		GtkTextView *view = l->data;
		check_visible_region_in_view (spell, view);
	}

#if ENABLE_DEBUG
	g_print ("%s() executed in %lf seconds\n",
		 G_STRFUNC,
		 g_timer_elapsed (timer, NULL));

	g_timer_destroy (timer);
#endif
}

static gboolean
timeout_cb (GspellInlineCheckerGtv *spell)
{
	check_visible_region (spell);

	spell->timeout_id = 0;
	return G_SOURCE_REMOVE;
}

static void
install_timeout (GspellInlineCheckerGtv *spell,
		 guint                   duration)
{
	if (spell->timeout_id != 0)
	{
		g_source_remove (spell->timeout_id);
	}

	spell->timeout_id = g_timeout_add (duration,
					   (GSourceFunc) timeout_cb,
					   spell);
}

static void
add_subregion_to_scan (GspellInlineCheckerGtv *spell,
		       const GtkTextIter      *start,
		       const GtkTextIter      *end)
{
	GtkTextIter start_adjusted;
	GtkTextIter end_adjusted;

	if (spell->scan_region == NULL)
	{
		spell->scan_region = gtk_text_region_new (spell->buffer);
	}

	start_adjusted = *start;
	end_adjusted = *end;
	adjust_iters_at_word_boundaries (&start_adjusted, &end_adjusted);

	gtk_text_region_add (spell->scan_region,
			     &start_adjusted,
			     &end_adjusted);
}

static void
recheck_all (GspellInlineCheckerGtv *spell)
{
	GtkTextIter start;
	GtkTextIter end;

	gtk_text_buffer_get_bounds (spell->buffer, &start, &end);

	add_subregion_to_scan (spell, &start, &end);
	check_visible_region (spell);
}

static void
insert_text_after_cb (GtkTextBuffer          *buffer,
		      GtkTextIter            *location,
		      gchar                  *text,
		      gint                    length,
		      GspellInlineCheckerGtv *spell)
{
	GtkTextIter start;
	GtkTextIter end;

	start = end = *location;
	gtk_text_iter_backward_chars (&start, g_utf8_strlen (text, length));

	add_subregion_to_scan (spell, &start, &end);
	install_timeout (spell, TIMEOUT_DURATION_BUFFER_MODIFIED);
}

static void
delete_range_after_cb (GtkTextBuffer          *buffer,
		       GtkTextIter            *start,
		       GtkTextIter            *end,
		       GspellInlineCheckerGtv *spell)
{
	add_subregion_to_scan (spell, start, end);
	install_timeout (spell, TIMEOUT_DURATION_BUFFER_MODIFIED);
}

static gboolean
get_word_extents_at_click_position (GspellInlineCheckerGtv *spell,
				    GtkTextIter            *start,
				    GtkTextIter            *end)
{
	GtkTextIter iter;

	gtk_text_buffer_get_iter_at_mark (spell->buffer, &iter, spell->mark_click);

	if (!gtk_text_iter_inside_word (&iter) &&
	    !gtk_text_iter_ends_word (&iter))
	{
		return FALSE;
	}

	*start = iter;
	if (!gtk_text_iter_starts_word (start))
	{
		gtk_text_iter_backward_word_start (start);
	}

	*end = iter;
	if (!gtk_text_iter_ends_word (end))
	{
		gtk_text_iter_forward_word_end (end);
	}

	return TRUE;
}

static void
add_to_dictionary_cb (GtkWidget              *menu_item,
		      GspellInlineCheckerGtv *spell)
{
	GtkTextIter start;
	GtkTextIter end;
	gchar *word;

	if (!get_word_extents_at_click_position (spell, &start, &end))
	{
		return;
	}

	word = gtk_text_buffer_get_text (spell->buffer, &start, &end, FALSE);

	gspell_checker_add_word_to_personal (spell->spell_checker, word);

	g_free (word);
}

static void
ignore_all_cb (GtkWidget              *menu_item,
	       GspellInlineCheckerGtv *spell)
{
	GtkTextIter start;
	GtkTextIter end;
	gchar *word;

	if (!get_word_extents_at_click_position (spell, &start, &end))
	{
		return;
	}

	word = gtk_text_buffer_get_text (spell->buffer, &start, &end, FALSE);

	gspell_checker_add_word_to_session (spell->spell_checker, word);

	g_free (word);
}

static void
replace_word_cb (GtkWidget              *menu_item,
		 GspellInlineCheckerGtv *spell)
{
	GtkTextIter start;
	GtkTextIter end;
	gchar *old_word;
	const gchar *new_word;

	if (!get_word_extents_at_click_position (spell, &start, &end))
	{
		return;
	}

	old_word = gtk_text_buffer_get_text (spell->buffer, &start, &end, FALSE);

	new_word = g_object_get_data (G_OBJECT (menu_item), SUGGESTION_KEY);
	g_return_if_fail (new_word != NULL);

	gtk_text_buffer_begin_user_action (spell->buffer);

	gtk_text_buffer_delete (spell->buffer, &start, &end);
	gtk_text_buffer_insert (spell->buffer, &start, new_word, -1);

	gtk_text_buffer_end_user_action (spell->buffer);

	gspell_checker_set_correction (spell->spell_checker, old_word, new_word);

	g_free (old_word);
}

static GtkWidget *
get_suggestion_menu (GspellInlineCheckerGtv *spell,
		     const gchar            *word)
{
	GtkWidget *top_menu;
	GtkWidget *menu_item;
	GSList *suggestions;

	top_menu = gtk_menu_new ();

	suggestions = gspell_checker_get_suggestions (spell->spell_checker, word);

	if (suggestions == NULL)
	{
		/* No suggestions. Put something in the menu anyway... */
		menu_item = gtk_menu_item_new_with_label (_("(no suggested words)"));
		gtk_widget_set_sensitive (menu_item, FALSE);
		gtk_menu_shell_prepend (GTK_MENU_SHELL (top_menu), menu_item);
	}
	else
	{
		GtkWidget *menu = top_menu;
		gint count = 0;
		GSList *l;

		/* Build a set of menus with suggestions. */
		for (l = suggestions; l != NULL; l = l->next)
		{
			gchar *suggested_word = l->data;
			GtkWidget *label;
			gchar *label_text;

			if (count == 10)
			{
				/* Separator */
				menu_item = gtk_separator_menu_item_new ();
				gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

				menu_item = gtk_menu_item_new_with_mnemonic (_("_More..."));
				gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

				menu = gtk_menu_new ();
				gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), menu);
				count = 0;
			}

			label_text = g_strdup_printf ("<b>%s</b>", suggested_word);

			label = gtk_label_new (label_text);
			gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
			gtk_widget_set_halign (label, GTK_ALIGN_START);

			menu_item = gtk_menu_item_new ();
			gtk_container_add (GTK_CONTAINER (menu_item), label);
			gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);

			g_object_set_data_full (G_OBJECT (menu_item),
						SUGGESTION_KEY,
						g_strdup (suggested_word),
						g_free);

			g_signal_connect (menu_item,
					  "activate",
					  G_CALLBACK (replace_word_cb),
					  spell);

			g_free (label_text);
			count++;
		}
	}

	g_slist_free_full (suggestions, g_free);

	/* Separator */
	menu_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), menu_item);

	/* Ignore all */
	menu_item = gtk_menu_item_new_with_mnemonic (_("_Ignore All"));
	gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), menu_item);

	g_signal_connect (menu_item,
			  "activate",
			  G_CALLBACK (ignore_all_cb),
			  spell);

	/* Add to Dictionary */
	menu_item = gtk_menu_item_new_with_mnemonic (_("_Add"));
	gtk_menu_shell_append (GTK_MENU_SHELL (top_menu), menu_item);

	g_signal_connect (menu_item,
			  "activate",
			  G_CALLBACK (add_to_dictionary_cb),
			  spell);

	return top_menu;
}

static void
populate_popup_cb (GtkTextView            *view,
		   GtkMenu                *menu,
		   GspellInlineCheckerGtv *spell)
{
	GtkWidget *menu_item;
	GtkTextIter start;
	GtkTextIter end;
	gchar *word;

	if (!get_word_extents_at_click_position (spell, &start, &end))
	{
		return;
	}

	if (!gtk_text_iter_has_tag (&start, spell->tag_highlight))
	{
		return;
	}

	/* Prepend separator */
	menu_item = gtk_separator_menu_item_new ();
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), menu_item);
	gtk_widget_show (menu_item);

	/* Prepend suggestions */
	menu_item = gtk_menu_item_new_with_mnemonic (_("_Spelling Suggestions..."));
	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), menu_item);

	word = gtk_text_buffer_get_text (spell->buffer, &start, &end, FALSE);
	gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item),
				   get_suggestion_menu (spell, word));
	g_free (word);

	gtk_widget_show_all (menu_item);
}

static gboolean
draw_cb (GtkWidget              *text_view,
	 cairo_t                *cr,
	 GspellInlineCheckerGtv *spell)
{
	install_timeout (spell, TIMEOUT_DURATION_DRAWING);

	return GDK_EVENT_PROPAGATE;
}

static void
remove_tag_to_word (GspellInlineCheckerGtv *spell,
		    const gchar            *word)
{
	GtkTextIter iter;

	gtk_text_buffer_get_start_iter (spell->buffer, &iter);

	while (TRUE)
	{
		gboolean found;
		GtkTextIter match_start;
		GtkTextIter match_end;

		found = gtk_text_iter_forward_search (&iter,
						      word,
						      GTK_TEXT_SEARCH_VISIBLE_ONLY |
						      GTK_TEXT_SEARCH_TEXT_ONLY,
						      &match_start,
						      &match_end,
						      NULL);

		if (!found)
		{
			break;
		}

		if (gtk_text_iter_starts_word (&match_start) &&
		    gtk_text_iter_ends_word (&match_end))
		{
			gtk_text_buffer_remove_tag (spell->buffer,
						    spell->tag_highlight,
						    &match_start,
						    &match_end);
		}

		iter = match_end;
	}
}

static void
add_word_cb (GspellChecker          *checker,
	     const gchar            *word,
	     GspellInlineCheckerGtv *spell)
{
	remove_tag_to_word (spell, word);
}

static void
clear_session_cb (GspellChecker          *checker,
		  GspellInlineCheckerGtv *spell)
{
	recheck_all (spell);
}

static void
language_notify_cb (GspellChecker          *checker,
		    GParamSpec             *pspec,
		    GspellInlineCheckerGtv *spell)
{
	recheck_all (spell);
}

/* When the user right-clicks on a word, they want to check that word.
 * Here, we do NOT move the cursor to the location of the clicked-upon word
 * since that prevents the use of edit functions on the context menu.
 */
static gboolean
button_press_event_cb (GtkTextView            *view,
		       GdkEventButton         *event,
		       GspellInlineCheckerGtv *spell)
{
	if (event->button == GDK_BUTTON_SECONDARY)
	{
		GtkTextBuffer *buffer = gtk_text_view_get_buffer (view);
		GtkTextIter iter;
		gint x;
		gint y;

		gtk_text_view_window_to_buffer_coords (view,
						       GTK_TEXT_WINDOW_TEXT,
						       event->x, event->y,
						       &x, &y);

		gtk_text_view_get_iter_at_location (view, &iter, x, y);

		gtk_text_buffer_move_mark (buffer, spell->mark_click, &iter);
	}

	return GDK_EVENT_PROPAGATE;
}

/* Move the insert mark before popping up the menu, otherwise it
 * will contain the wrong set of suggestions.
 */
static gboolean
popup_menu_cb (GtkTextView            *view,
	       GspellInlineCheckerGtv *spell)
{
	GtkTextIter iter;

	gtk_text_buffer_get_iter_at_mark (spell->buffer, &iter,
					  gtk_text_buffer_get_insert (spell->buffer));
	gtk_text_buffer_move_mark (spell->buffer, spell->mark_click, &iter);

	return FALSE;
}

static void
update_tag_highlight_priority (GspellInlineCheckerGtv *spell,
			       GtkTextTagTable        *table)
{
	g_return_if_fail (spell->tag_highlight != NULL);

	gtk_text_tag_set_priority (spell->tag_highlight,
				   gtk_text_tag_table_get_size (table) - 1);
}

static void
tag_added_cb (GtkTextTagTable        *table,
	      GtkTextTag             *tag,
	      GspellInlineCheckerGtv *spell)
{
	update_tag_highlight_priority (spell, table);
}

static void
tag_removed_cb (GtkTextTagTable        *table,
		GtkTextTag             *tag,
		GspellInlineCheckerGtv *spell)
{
	if (tag != spell->tag_highlight)
	{
		update_tag_highlight_priority (spell, table);
	}
}

static void
tag_changed_cb (GtkTextTagTable        *table,
		GtkTextTag             *tag,
		gboolean                size_changed,
		GspellInlineCheckerGtv *spell)
{
	update_tag_highlight_priority (spell, table);
}

static void
highlight_updated_cb (GtkSourceBuffer        *buffer,
		      GtkTextIter            *start,
		      GtkTextIter            *end,
		      GspellInlineCheckerGtv *spell)
{
	add_subregion_to_scan (spell, start, end);
	install_timeout (spell, TIMEOUT_DURATION_BUFFER_MODIFIED);
}

static void
set_buffer (GspellInlineCheckerGtv *spell,
	    GtkSourceBuffer        *buffer)
{
	GtkTextTagTable *tag_table;
	GtkTextIter start;

	g_return_if_fail (GTK_SOURCE_IS_BUFFER (buffer));
	g_return_if_fail (spell->buffer == NULL);
	g_return_if_fail (spell->tag_highlight == NULL);
	g_return_if_fail (spell->mark_click == NULL);

	spell->buffer = g_object_ref (buffer);

	g_object_set_data (G_OBJECT (buffer),
			   INLINE_CHECKER_GTV_KEY,
			   spell);

	g_signal_connect_object (buffer,
				 "insert-text",
				 G_CALLBACK (insert_text_after_cb),
				 spell,
				 G_CONNECT_AFTER);

	g_signal_connect_object (buffer,
				 "delete-range",
				 G_CALLBACK (delete_range_after_cb),
				 spell,
				 G_CONNECT_AFTER);

	g_signal_connect_object (buffer,
				 "highlight-updated", /* GtkSourceBuffer signal */
				 G_CALLBACK (highlight_updated_cb),
				 spell,
				 0);

	spell->tag_highlight = gtk_text_buffer_create_tag (spell->buffer, NULL,
							   "underline", PANGO_UNDERLINE_ERROR,
							   NULL);
	g_object_ref (spell->tag_highlight);

	tag_table = gtk_text_buffer_get_tag_table (spell->buffer);

	g_signal_connect_object (tag_table,
				 "tag-added",
				 G_CALLBACK (tag_added_cb),
				 spell,
				 0);

	g_signal_connect_object (tag_table,
				 "tag-removed",
				 G_CALLBACK (tag_removed_cb),
				 spell,
				 0);

	g_signal_connect_object (tag_table,
				 "tag-changed",
				 G_CALLBACK (tag_changed_cb),
				 spell,
				 0);

	/* For now we don't care where the mark points. The start looks like a
	 * good place to begin with.
	 */
	gtk_text_buffer_get_start_iter (spell->buffer, &start);
	spell->mark_click = gtk_text_buffer_create_mark (spell->buffer, NULL, &start, TRUE);

	recheck_all (spell);

	g_object_notify (G_OBJECT (spell), "buffer");
}

static void
set_spell_checker (GspellInlineCheckerGtv *spell,
		   GspellChecker          *checker)
{
	g_return_if_fail (GSPELL_IS_CHECKER (checker));
	g_return_if_fail (spell->spell_checker == NULL);

	spell->spell_checker = g_object_ref (checker);

	g_signal_connect_object (spell->spell_checker,
				 "add_word_to_session",
				 G_CALLBACK (add_word_cb),
				 spell,
				 0);

	g_signal_connect_object (spell->spell_checker,
				 "add_word_to_personal",
				 G_CALLBACK (add_word_cb),
				 spell,
				 0);

	g_signal_connect_object (spell->spell_checker,
				 "clear_session",
				 G_CALLBACK (clear_session_cb),
				 spell,
				 0);

	g_signal_connect_object (spell->spell_checker,
				 "notify::language",
				 G_CALLBACK (language_notify_cb),
				 spell,
				 0);

	g_object_notify (G_OBJECT (spell), "spell-checker");
}

static void
gspell_inline_checker_gtv_get_property (GObject    *object,
					guint       prop_id,
					GValue     *value,
					GParamSpec *pspec)
{
	GspellInlineCheckerGtv *spell = GSPELL_INLINE_CHECKER_GTV (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_value_set_object (value, spell->buffer);
			break;

		case PROP_SPELL_CHECKER:
			g_value_set_object (value, spell->spell_checker);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_inline_checker_gtv_set_property (GObject      *object,
					guint         prop_id,
					const GValue *value,
					GParamSpec   *pspec)
{
	GspellInlineCheckerGtv *spell = GSPELL_INLINE_CHECKER_GTV (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			set_buffer (spell, g_value_get_object (value));
			break;

		case PROP_SPELL_CHECKER:
			set_spell_checker (spell, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
gspell_inline_checker_gtv_dispose (GObject *object)
{
	GspellInlineCheckerGtv *spell = GSPELL_INLINE_CHECKER_GTV (object);

	if (spell->buffer != NULL)
	{
		GtkTextTagTable *table;

		table = gtk_text_buffer_get_tag_table (spell->buffer);

		if (table != NULL && spell->tag_highlight != NULL)
		{
			gtk_text_tag_table_remove (table, spell->tag_highlight);
		}

		if (spell->mark_click != NULL)
		{
			gtk_text_buffer_delete_mark (spell->buffer, spell->mark_click);
			spell->mark_click = NULL;
		}

		g_object_set_data (G_OBJECT (spell->buffer), INLINE_CHECKER_GTV_KEY, NULL);

		g_object_unref (spell->buffer);
		spell->buffer = NULL;
	}

	g_clear_object (&spell->tag_highlight);
	g_clear_object (&spell->spell_checker);

	g_slist_free_full (spell->views, g_object_unref);
	spell->views = NULL;

	spell->mark_click = NULL;

	if (spell->scan_region != NULL)
	{
		gtk_text_region_destroy (spell->scan_region);
		spell->scan_region = NULL;
	}

	if (spell->timeout_id != 0)
	{
		g_source_remove (spell->timeout_id);
		spell->timeout_id = 0;
	}

	G_OBJECT_CLASS (gspell_inline_checker_gtv_parent_class)->dispose (object);
}

static void
gspell_inline_checker_gtv_class_init (GspellInlineCheckerGtvClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gspell_inline_checker_gtv_get_property;
	object_class->set_property = gspell_inline_checker_gtv_set_property;
	object_class->dispose = gspell_inline_checker_gtv_dispose;

	/**
	 * GspellInlineCheckerGtv:buffer:
	 *
	 * The #GtkSourceBuffer. If a same buffer is used for several views, the
	 * misspelled words are visible in all views, because #GtkTextTag's are
	 * added to the buffer.
	 */
	g_object_class_install_property (object_class,
					 PROP_BUFFER,
					 g_param_spec_object ("buffer",
							      "Buffer",
							      "",
							      GTK_SOURCE_TYPE_BUFFER,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));

	/**
	 * GspellInlineCheckerGtv:spell-checker:
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
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));
}

static void
gspell_inline_checker_gtv_init (GspellInlineCheckerGtv *spell)
{
}

/**
 * gspell_inline_checker_gtv_new:
 * @buffer: a #GtkSourceBuffer.
 * @checker: a #GspellChecker.
 *
 * Returns: a new #GspellInlineCheckerGtv object.
 */
GspellInlineCheckerGtv *
gspell_inline_checker_gtv_new (GtkSourceBuffer *buffer,
			       GspellChecker   *checker)
{
	GspellInlineCheckerGtv *spell;

	g_return_val_if_fail (GTK_SOURCE_IS_BUFFER (buffer), NULL);
	g_return_val_if_fail (GSPELL_IS_CHECKER (checker), NULL);

	spell = g_object_get_data (G_OBJECT (buffer), INLINE_CHECKER_GTV_KEY);
	if (spell != NULL)
	{
		g_object_ref (spell);
		g_return_val_if_fail (spell->spell_checker == checker, spell);
		return spell;
	}

	return g_object_new (GSPELL_TYPE_INLINE_CHECKER_GTV,
			     "buffer", buffer,
			     "spell-checker", checker,
			     NULL);
}

/**
 * gspell_inline_checker_gtv_attach_view:
 * @spell: a #GspellInlineCheckerGtv.
 * @view: a #GtkTextView.
 *
 * Enables the inline spell checker for @view. @view must have the same buffer as
 * the #GspellInlineCheckerGtv:buffer property.
 */
void
gspell_inline_checker_gtv_attach_view (GspellInlineCheckerGtv *spell,
				       GtkTextView            *view)
{
	g_return_if_fail (GSPELL_IS_INLINE_CHECKER_GTV (spell));
	g_return_if_fail (GTK_IS_TEXT_VIEW (view));
	g_return_if_fail (gtk_text_view_get_buffer (view) == spell->buffer);
	g_return_if_fail (g_slist_find (spell->views, view) == NULL);

	g_signal_connect_object (view,
				 "button-press-event",
				 G_CALLBACK (button_press_event_cb),
				 spell,
				 0);

	g_signal_connect_object (view,
				 "popup-menu",
				 G_CALLBACK (popup_menu_cb),
				 spell,
				 0);

	g_signal_connect_object (view,
				 "populate-popup",
				 G_CALLBACK (populate_popup_cb),
				 spell,
				 0);

	g_signal_connect_object (view,
				 "draw",
				 G_CALLBACK (draw_cb),
				 spell,
				 0);

	spell->views = g_slist_prepend (spell->views, view);
	g_object_ref (view);

	check_visible_region_in_view (spell, view);
}

/**
 * gspell_inline_checker_gtv_detach_view:
 * @spell: a #GspellInlineCheckerGtv.
 * @view: a #GtkTextView.
 *
 * Disables the inline spell checker for @view.
 */
void
gspell_inline_checker_gtv_detach_view (GspellInlineCheckerGtv *spell,
				       GtkTextView            *view)
{
	g_return_if_fail (GSPELL_IS_INLINE_CHECKER_GTV (spell));
	g_return_if_fail (GTK_IS_TEXT_VIEW (view));
	g_return_if_fail (gtk_text_view_get_buffer (view) == spell->buffer);
	g_return_if_fail (g_slist_find (spell->views, view) != NULL);

	g_signal_handlers_disconnect_by_data (view, spell);

	spell->views = g_slist_remove (spell->views, view);
	g_object_unref (view);
}

/* ex:set ts=8 noet: */
