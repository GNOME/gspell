/*
 * This file is part of gspell, a spell-checking library.
 *
 * Copyright 2002 - Paolo Maggi
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

/* This is a modified version of GtkSpell 2.0.5 (gtkspell.sf.net)
 * Copyright 2002 - Evan Martin
 */

#include "config.h"
#include "gspell-inline-checker-text-buffer.h"
#include <string.h>
#include <glib/gi18n-lib.h>
#include "gspell-checker.h"
#include "gspell-buffer-notifier.h"
#include "gspell-text-buffer.h"
#include "gspell-text-region.h"
#include "gspell-utils.h"

struct _GspellInlineCheckerTextBuffer
{
	GObject parent;

	GtkTextBuffer *buffer;
	GspellChecker *spell_checker;

	/* List of GtkTextView* */
	GSList *views;

	GtkTextTag *highlight_tag;
	GtkTextTag *no_spell_check_tag;

	GtkTextMark *mark_click;

	GspellTextRegion *scan_region;
	guint timeout_id;

	/* If the unit test mode is enabled, there is no timeouts, and the whole
	 * buffer is scanned synchronously.
	 * The unit test mode tries to follow as most as possible the same code
	 * paths as the real code paths, otherwise the unit tests would be
	 * useless. As such, the function names reflect the real code paths.
	 */
	guint unit_test_mode : 1;
};

enum
{
	PROP_0,
	PROP_BUFFER,
};

#define INLINE_CHECKER_TEXT_BUFFER_KEY	"GspellInlineCheckerTextBufferID"
#define SUGGESTION_KEY			"GspellInlineSuggestionID"

/* Timeout durations in milliseconds. Writing and deleting text should be smooth
 * and responsive.
 */
#define TIMEOUT_DURATION_BUFFER_MODIFIED 400
#define TIMEOUT_DURATION_DRAWING 20

G_DEFINE_TYPE (GspellInlineCheckerTextBuffer, _gspell_inline_checker_text_buffer, G_TYPE_OBJECT)

static void
check_word (GspellInlineCheckerTextBuffer *spell,
	    const GtkTextIter             *start,
	    const GtkTextIter             *end)
{
	gchar *word;
	GError *error = NULL;
	gboolean correctly_spelled;

	if (spell->spell_checker == NULL ||
	    gspell_checker_get_language (spell->spell_checker) == NULL)
	{
		return;
	}

	if (!gtk_text_iter_starts_word (start) ||
	    !gtk_text_iter_ends_word (end))
	{
		g_warning ("Spell checking: @start and @end must delimit a word");
		return;
	}

	word = gtk_text_buffer_get_text (spell->buffer, start, end, FALSE);

	correctly_spelled = gspell_checker_check_word (spell->spell_checker,
						       word, -1,
						       &error);

	if (error != NULL)
	{
		g_warning ("Inline spell checker: %s", error->message);
		g_clear_error (&error);
	}

	if (!correctly_spelled)
	{
		gtk_text_buffer_apply_tag (spell->buffer,
					   spell->highlight_tag,
					   start,
					   end);
	}

	g_free (word);
}

static void
check_subregion (GspellInlineCheckerTextBuffer *spell,
		 GtkTextIter                   *start,
		 GtkTextIter                   *end)
{
	GtkTextIter word_start;

	g_assert_cmpint (gtk_text_iter_compare (start, end), <=, 0);

	if (gtk_text_iter_inside_word (start) &&
	    !gtk_text_iter_starts_word (start))
	{
		gtk_text_iter_backward_word_start (start);
	}

	if (gtk_text_iter_inside_word (end) &&
	    !gtk_text_iter_starts_word (end))
	{
		gtk_text_iter_forward_word_end (end);
	}

	gtk_text_buffer_remove_tag (spell->buffer,
				    spell->highlight_tag,
				    start,
				    end);

	word_start = *start;
	if (!gtk_text_iter_starts_word (&word_start))
	{
		gtk_text_iter_forward_word_end (&word_start);

		/* Didn't move, there is no words after @start_adjusted. */
		if (gtk_text_iter_equal (&word_start, start))
		{
			return;
		}

		gtk_text_iter_backward_word_start (&word_start);
		g_assert (gtk_text_iter_starts_word (&word_start));
		g_assert_cmpint (gtk_text_iter_compare (start, &word_start), <, 0);
	}

	while (_gspell_utils_skip_no_spell_check (spell->no_spell_check_tag, &word_start, end) &&
	       gtk_text_iter_compare (&word_start, end) < 0)
	{
		GtkTextIter word_end;
		GtkTextIter next_word_start;

		g_assert (gtk_text_iter_starts_word (&word_start));

		word_end = word_start;
		gtk_text_iter_forward_word_end (&word_end);

		g_assert_cmpint (gtk_text_iter_compare (&word_end, end), <=, 0);

		check_word (spell, &word_start, &word_end);

		next_word_start = word_end;
		gtk_text_iter_forward_word_end (&next_word_start);
		gtk_text_iter_backward_word_start (&next_word_start);

		/* Make sure we've actually advanced (we don't advance if we
		 * have just checked the last word of the buffer).
		 */
		if (gtk_text_iter_compare (&next_word_start, &word_start) <= 0)
		{
			break;
		}

		word_start = next_word_start;
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
 * When calling text_region_add() with equal iters, the subregion is not
 * added. But when a subregion becomes empty, due to text deletion, the
 * subregion is not removed from the TextRegion.
 */
static gboolean
is_text_region_empty (GspellTextRegion *region)
{
	GspellTextRegionIterator region_iter;

	if (region == NULL)
	{
		return TRUE;
	}

	_gspell_text_region_get_iterator (region, &region_iter, 0);

	while (!_gspell_text_region_iterator_is_end (&region_iter))
	{
		GtkTextIter region_start;
		GtkTextIter region_end;

		if (!_gspell_text_region_iterator_get_subregion (&region_iter,
								 &region_start,
								 &region_end))
		{
			return TRUE;
		}

		if (!gtk_text_iter_equal (&region_start, &region_end))
		{
			return FALSE;
		}

		_gspell_text_region_iterator_next (&region_iter);
	}

	return TRUE;
}

static void
check_visible_region_in_view (GspellInlineCheckerTextBuffer *spell,
			      GtkTextView                   *view)
{
	GtkTextIter visible_start;
	GtkTextIter visible_end;
	GspellTextRegion *intersect;
	GspellTextRegionIterator intersect_iter;

	if (spell->scan_region == NULL)
	{
		return;
	}

	if (view != NULL)
	{
		get_visible_region (view, &visible_start, &visible_end);
	}
	else
	{
		g_assert_true (spell->unit_test_mode);
		gtk_text_buffer_get_bounds (spell->buffer, &visible_start, &visible_end);
	}

	intersect = _gspell_text_region_intersect (spell->scan_region,
						   &visible_start,
						   &visible_end);

	if (intersect == NULL)
	{
		return;
	}

	_gspell_text_region_get_iterator (intersect, &intersect_iter, 0);

	while (!_gspell_text_region_iterator_is_end (&intersect_iter))
	{
		GtkTextIter start;
		GtkTextIter end;

		if (!_gspell_text_region_iterator_get_subregion (&intersect_iter,
								 &start,
								 &end))
		{
			return;
		}

		check_subregion (spell, &start, &end);

		_gspell_text_region_subtract (spell->scan_region, &start, &end);

		_gspell_text_region_iterator_next (&intersect_iter);
	}

	_gspell_text_region_destroy (intersect);

	if (is_text_region_empty (spell->scan_region))
	{
		_gspell_text_region_destroy (spell->scan_region);
		spell->scan_region = NULL;
	}
}

static void
check_visible_region (GspellInlineCheckerTextBuffer *spell)
{
	GSList *l;

	if (spell->scan_region == NULL)
	{
		return;
	}

	if (spell->unit_test_mode)
	{
		check_visible_region_in_view (spell, NULL);
		return;
	}

	for (l = spell->views; l != NULL; l = l->next)
	{
		GtkTextView *view = GTK_TEXT_VIEW (l->data);
		check_visible_region_in_view (spell, view);
	}
}

static gboolean
timeout_cb (GspellInlineCheckerTextBuffer *spell)
{
	check_visible_region (spell);

	spell->timeout_id = 0;
	return G_SOURCE_REMOVE;
}

static void
install_timeout (GspellInlineCheckerTextBuffer *spell,
		 guint                          duration)
{
	if (spell->timeout_id != 0)
	{
		g_source_remove (spell->timeout_id);
		spell->timeout_id = 0;
	}

	if (spell->unit_test_mode)
	{
		timeout_cb (spell);
	}
	else
	{
		spell->timeout_id = g_timeout_add (duration,
						   (GSourceFunc) timeout_cb,
						   spell);
	}
}

static void
add_subregion_to_scan (GspellInlineCheckerTextBuffer *spell,
		       const GtkTextIter             *start,
		       const GtkTextIter             *end)
{
	if (spell->scan_region == NULL)
	{
		spell->scan_region = _gspell_text_region_new (spell->buffer);
	}

	_gspell_text_region_add (spell->scan_region, start, end);
}

static void
recheck_all (GspellInlineCheckerTextBuffer *spell)
{
	GtkTextIter start;
	GtkTextIter end;

	gtk_text_buffer_get_bounds (spell->buffer, &start, &end);

	add_subregion_to_scan (spell, &start, &end);
	check_visible_region (spell);
}

static void
insert_text_after_cb (GtkTextBuffer                 *buffer,
		      GtkTextIter                   *location,
		      gchar                         *text,
		      gint                           length,
		      GspellInlineCheckerTextBuffer *spell)
{
	GtkTextIter start;
	GtkTextIter end;

	start = end = *location;
	gtk_text_iter_backward_chars (&start, g_utf8_strlen (text, length));

	add_subregion_to_scan (spell, &start, &end);
	install_timeout (spell, TIMEOUT_DURATION_BUFFER_MODIFIED);
}

static void
delete_range_after_cb (GtkTextBuffer                 *buffer,
		       GtkTextIter                   *start,
		       GtkTextIter                   *end,
		       GspellInlineCheckerTextBuffer *spell)
{
	GtkTextIter start_adjusted;
	GtkTextIter end_adjusted;

	start_adjusted = *start;
	end_adjusted = *end;

	/* Just to be sure. Normally start == end. */
	gtk_text_iter_order (&start_adjusted, &end_adjusted);

	if (gtk_text_iter_ends_word (&start_adjusted) ||
	    (gtk_text_iter_inside_word (&start_adjusted) &&
	     !gtk_text_iter_starts_word (&start_adjusted)))
	{
		gtk_text_iter_backward_word_start (&start_adjusted);
	}

	if (gtk_text_iter_inside_word (&end_adjusted))
	{
		gtk_text_iter_forward_word_end (&end_adjusted);
	}

	add_subregion_to_scan (spell, &start_adjusted, &end_adjusted);
	install_timeout (spell, TIMEOUT_DURATION_BUFFER_MODIFIED);
}

static gboolean
get_word_extents_at_click_position (GspellInlineCheckerTextBuffer *spell,
				    GtkTextIter                   *start,
				    GtkTextIter                   *end)
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
add_to_dictionary_cb (GtkWidget                     *menu_item,
		      GspellInlineCheckerTextBuffer *spell)
{
	GtkTextIter start;
	GtkTextIter end;
	gchar *word;

	if (!get_word_extents_at_click_position (spell, &start, &end))
	{
		return;
	}

	word = gtk_text_buffer_get_text (spell->buffer, &start, &end, FALSE);

	if (spell->spell_checker != NULL)
	{
		gspell_checker_add_word_to_personal (spell->spell_checker, word, -1);
	}

	g_free (word);
}

static void
ignore_all_cb (GtkWidget                     *menu_item,
	       GspellInlineCheckerTextBuffer *spell)
{
	GtkTextIter start;
	GtkTextIter end;
	gchar *word;

	if (!get_word_extents_at_click_position (spell, &start, &end))
	{
		return;
	}

	word = gtk_text_buffer_get_text (spell->buffer, &start, &end, FALSE);

	if (spell->spell_checker != NULL)
	{
		gspell_checker_add_word_to_session (spell->spell_checker, word, -1);
	}

	g_free (word);
}

static void
replace_word_cb (GtkWidget                     *menu_item,
		 GspellInlineCheckerTextBuffer *spell)
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

	if (spell->spell_checker != NULL)
	{
		gspell_checker_set_correction (spell->spell_checker,
					       old_word, -1,
					       new_word, -1);
	}

	g_free (old_word);
}

static GtkWidget *
get_suggestion_menu (GspellInlineCheckerTextBuffer *spell,
		     const gchar                   *word)
{
	GtkWidget *top_menu;
	GtkWidget *menu_item;
	GSList *suggestions = NULL;

	top_menu = gtk_menu_new ();

	if (spell->spell_checker != NULL)
	{
		suggestions = gspell_checker_get_suggestions (spell->spell_checker, word, -1);
	}

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
populate_popup_cb (GtkTextView                   *view,
		   GtkMenu                       *menu,
		   GspellInlineCheckerTextBuffer *spell)
{
	GtkWidget *menu_item;
	GtkTextIter start;
	GtkTextIter end;
	gchar *word;

	if (!get_word_extents_at_click_position (spell, &start, &end))
	{
		return;
	}

	if (!gtk_text_iter_has_tag (&start, spell->highlight_tag))
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
draw_cb (GtkWidget                     *text_view,
	 cairo_t                       *cr,
	 GspellInlineCheckerTextBuffer *spell)
{
	install_timeout (spell, TIMEOUT_DURATION_DRAWING);

	return GDK_EVENT_PROPAGATE;
}

static void
remove_tag_to_word (GspellInlineCheckerTextBuffer *spell,
		    const gchar                   *word)
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
						    spell->highlight_tag,
						    &match_start,
						    &match_end);
		}

		iter = match_end;
	}
}

static void
word_added_cb (GspellChecker                 *checker,
	       const gchar                   *word,
	       GspellInlineCheckerTextBuffer *spell)
{
	remove_tag_to_word (spell, word);
}

static void
session_cleared_cb (GspellChecker                 *checker,
		    GspellInlineCheckerTextBuffer *spell)
{
	recheck_all (spell);
}

static void
language_notify_cb (GspellChecker                 *checker,
		    GParamSpec                    *pspec,
		    GspellInlineCheckerTextBuffer *spell)
{
	recheck_all (spell);
}

/* When the user right-clicks on a word, they want to check that word.
 * Here, we do NOT move the cursor to the location of the clicked-upon word
 * since that prevents the use of edit functions on the context menu.
 */
static gboolean
button_press_event_cb (GtkTextView                   *view,
		       GdkEventButton                *event,
		       GspellInlineCheckerTextBuffer *spell)
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
popup_menu_cb (GtkTextView                   *view,
	       GspellInlineCheckerTextBuffer *spell)
{
	GtkTextIter iter;

	gtk_text_buffer_get_iter_at_mark (spell->buffer, &iter,
					  gtk_text_buffer_get_insert (spell->buffer));
	gtk_text_buffer_move_mark (spell->buffer, spell->mark_click, &iter);

	return FALSE;
}

static void
apply_or_remove_tag_cb (GtkTextBuffer                 *buffer,
			GtkTextTag                    *tag,
			GtkTextIter                   *start,
			GtkTextIter                   *end,
			GspellInlineCheckerTextBuffer *spell)
{
	if (spell->no_spell_check_tag != NULL &&
	    spell->no_spell_check_tag == tag)
	{
		add_subregion_to_scan (spell, start, end);
		install_timeout (spell, TIMEOUT_DURATION_BUFFER_MODIFIED);
	}
}

static void
tag_added_cb (GtkTextTagTable               *table,
	      GtkTextTag                    *tag,
	      GspellInlineCheckerTextBuffer *spell)
{
	gchar *name;

	g_object_get (tag, "name", &name, NULL);

	if (g_strcmp0 (name, "gtksourceview:context-classes:no-spell-check") == 0)
	{
		g_return_if_fail (spell->no_spell_check_tag == NULL);

		spell->no_spell_check_tag = g_object_ref (tag);

		recheck_all (spell);
	}

	g_free (name);
}

static void
tag_removed_cb (GtkTextTagTable               *table,
		GtkTextTag                    *tag,
		GspellInlineCheckerTextBuffer *spell)
{
	if (spell->no_spell_check_tag != NULL &&
	    spell->no_spell_check_tag == tag)
	{
		g_clear_object (&spell->no_spell_check_tag);

		recheck_all (spell);
	}
}

static void
set_spell_checker (GspellInlineCheckerTextBuffer *spell,
		   GspellChecker                 *checker)
{
	g_return_if_fail (checker == NULL || GSPELL_IS_CHECKER (checker));

	if (spell->spell_checker == checker)
	{
		return;
	}

	if (spell->spell_checker != NULL)
	{
		g_signal_handlers_disconnect_by_data (spell->spell_checker, spell);
		g_object_unref (spell->spell_checker);
	}

	spell->spell_checker = checker;

	if (spell->spell_checker != NULL)
	{
		g_object_ref (spell->spell_checker);

		g_signal_connect (spell->spell_checker,
				  "word-added-to-session",
				  G_CALLBACK (word_added_cb),
				  spell);

		g_signal_connect (spell->spell_checker,
				  "word-added-to-personal",
				  G_CALLBACK (word_added_cb),
				  spell);

		g_signal_connect (spell->spell_checker,
				  "session-cleared",
				  G_CALLBACK (session_cleared_cb),
				  spell);

		g_signal_connect (spell->spell_checker,
				  "notify::language",
				  G_CALLBACK (language_notify_cb),
				  spell);
	}
}

static void
text_buffer_checker_changed_cb (GspellBufferNotifier          *notifier,
				GtkTextBuffer                 *buffer,
				GspellChecker                 *new_checker,
				GspellInlineCheckerTextBuffer *spell)
{
	if (spell->buffer == buffer)
	{
		set_spell_checker (spell, new_checker);
		recheck_all (spell);
	}
}

static void
set_buffer (GspellInlineCheckerTextBuffer *spell,
	    GtkTextBuffer                 *buffer)
{
	GtkTextTagTable *tag_table;
	GtkTextIter start;
	GspellChecker *checker;
	GspellBufferNotifier *notifier;

	g_return_if_fail (GTK_IS_TEXT_BUFFER (buffer));
	g_return_if_fail (spell->buffer == NULL);
	g_return_if_fail (spell->highlight_tag == NULL);
	g_return_if_fail (spell->no_spell_check_tag == NULL);
	g_return_if_fail (spell->mark_click == NULL);

	spell->buffer = g_object_ref (buffer);

	g_object_set_data (G_OBJECT (buffer),
			   INLINE_CHECKER_TEXT_BUFFER_KEY,
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
				 "apply-tag",
				 G_CALLBACK (apply_or_remove_tag_cb),
				 spell,
				 G_CONNECT_AFTER);

	g_signal_connect_object (buffer,
				 "remove-tag",
				 G_CALLBACK (apply_or_remove_tag_cb),
				 spell,
				 G_CONNECT_AFTER);

	spell->highlight_tag = gtk_text_buffer_create_tag (spell->buffer, NULL,
							   "underline", PANGO_UNDERLINE_ERROR,
							   NULL);
	g_object_ref (spell->highlight_tag);

	spell->no_spell_check_tag = _gspell_utils_get_no_spell_check_tag (spell->buffer);
	if (spell->no_spell_check_tag != NULL)
	{
		g_object_ref (spell->no_spell_check_tag);
	}

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

	/* For now we don't care where the mark points. The start looks like a
	 * good place to begin with.
	 */
	gtk_text_buffer_get_start_iter (spell->buffer, &start);
	spell->mark_click = gtk_text_buffer_create_mark (spell->buffer, NULL, &start, TRUE);

	checker = gspell_text_buffer_get_spell_checker (spell->buffer);
	set_spell_checker (spell, checker);

	notifier = _gspell_buffer_notifier_get_instance ();
	g_signal_connect_object (notifier,
				 "text-buffer-checker-changed",
				 G_CALLBACK (text_buffer_checker_changed_cb),
				 spell,
				 0);

	recheck_all (spell);

	g_object_notify (G_OBJECT (spell), "buffer");
}

static void
_gspell_inline_checker_text_buffer_get_property (GObject    *object,
						 guint       prop_id,
						 GValue     *value,
						 GParamSpec *pspec)
{
	GspellInlineCheckerTextBuffer *spell = GSPELL_INLINE_CHECKER_TEXT_BUFFER (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			g_value_set_object (value, spell->buffer);
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
_gspell_inline_checker_text_buffer_set_property (GObject      *object,
						 guint         prop_id,
						 const GValue *value,
						 GParamSpec   *pspec)
{
	GspellInlineCheckerTextBuffer *spell = GSPELL_INLINE_CHECKER_TEXT_BUFFER (object);

	switch (prop_id)
	{
		case PROP_BUFFER:
			set_buffer (spell, g_value_get_object (value));
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void
_gspell_inline_checker_text_buffer_dispose (GObject *object)
{
	GspellInlineCheckerTextBuffer *spell = GSPELL_INLINE_CHECKER_TEXT_BUFFER (object);

	if (spell->buffer != NULL)
	{
		GtkTextTagTable *table;

		table = gtk_text_buffer_get_tag_table (spell->buffer);

		if (table != NULL && spell->highlight_tag != NULL)
		{
			gtk_text_tag_table_remove (table, spell->highlight_tag);
		}

		if (spell->mark_click != NULL)
		{
			gtk_text_buffer_delete_mark (spell->buffer, spell->mark_click);
			spell->mark_click = NULL;
		}

		g_object_set_data (G_OBJECT (spell->buffer), INLINE_CHECKER_TEXT_BUFFER_KEY, NULL);

		g_object_unref (spell->buffer);
		spell->buffer = NULL;
	}

	set_spell_checker (spell, NULL);

	g_clear_object (&spell->highlight_tag);
	g_clear_object (&spell->no_spell_check_tag);

	g_slist_free (spell->views);
	spell->views = NULL;

	spell->mark_click = NULL;

	if (spell->scan_region != NULL)
	{
		_gspell_text_region_destroy (spell->scan_region);
		spell->scan_region = NULL;
	}

	if (spell->timeout_id != 0)
	{
		g_source_remove (spell->timeout_id);
		spell->timeout_id = 0;
	}

	G_OBJECT_CLASS (_gspell_inline_checker_text_buffer_parent_class)->dispose (object);
}

static void
_gspell_inline_checker_text_buffer_class_init (GspellInlineCheckerTextBufferClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = _gspell_inline_checker_text_buffer_get_property;
	object_class->set_property = _gspell_inline_checker_text_buffer_set_property;
	object_class->dispose = _gspell_inline_checker_text_buffer_dispose;

	g_object_class_install_property (object_class,
					 PROP_BUFFER,
					 g_param_spec_object ("buffer",
							      "Buffer",
							      "",
							      GTK_TYPE_TEXT_BUFFER,
							      G_PARAM_READWRITE |
							      G_PARAM_CONSTRUCT_ONLY |
							      G_PARAM_STATIC_STRINGS));
}

static void
_gspell_inline_checker_text_buffer_init (GspellInlineCheckerTextBuffer *spell)
{
}

GspellInlineCheckerTextBuffer *
_gspell_inline_checker_text_buffer_new (GtkTextBuffer *buffer)
{
	GspellInlineCheckerTextBuffer *spell;

	g_return_val_if_fail (GTK_IS_TEXT_BUFFER (buffer), NULL);

	spell = g_object_get_data (G_OBJECT (buffer), INLINE_CHECKER_TEXT_BUFFER_KEY);
	if (spell != NULL)
	{
		return g_object_ref (spell);
	}

	return g_object_new (GSPELL_TYPE_INLINE_CHECKER_TEXT_BUFFER,
			     "buffer", buffer,
			     NULL);
}

/**
 * _gspell_inline_checker_text_buffer_attach_view:
 * @spell: a #GspellInlineCheckerTextBuffer.
 * @view: a #GtkTextView.
 *
 * Enables the inline spell checker for @view. @view must have the same buffer as
 * the #GspellInlineCheckerTextBuffer:buffer property.
 */
void
_gspell_inline_checker_text_buffer_attach_view (GspellInlineCheckerTextBuffer *spell,
						GtkTextView                   *view)
{
	g_return_if_fail (GSPELL_IS_INLINE_CHECKER_TEXT_BUFFER (spell));
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

	check_visible_region_in_view (spell, view);
}

/**
 * _gspell_inline_checker_text_buffer_detach_view:
 * @spell: a #GspellInlineCheckerTextBuffer.
 * @view: a #GtkTextView.
 *
 * Disables the inline spell checker for @view.
 */
void
_gspell_inline_checker_text_buffer_detach_view (GspellInlineCheckerTextBuffer *spell,
						GtkTextView                   *view)
{
	g_return_if_fail (GSPELL_IS_INLINE_CHECKER_TEXT_BUFFER (spell));
	g_return_if_fail (GTK_IS_TEXT_VIEW (view));
	g_return_if_fail (g_slist_find (spell->views, view) != NULL);

	g_signal_handlers_disconnect_by_data (view, spell);

	spell->views = g_slist_remove (spell->views, view);
}

void
_gspell_inline_checker_text_buffer_set_unit_test_mode (GspellInlineCheckerTextBuffer *spell,
						       gboolean                       unit_test_mode)
{
	g_return_if_fail (GSPELL_IS_INLINE_CHECKER_TEXT_BUFFER (spell));

	spell->unit_test_mode = unit_test_mode != FALSE;

	if (spell->timeout_id != 0)
	{
		g_source_remove (spell->timeout_id);
		spell->timeout_id = 0;
		timeout_cb (spell);
	}

	check_visible_region (spell);
}

GtkTextTag *
_gspell_inline_checker_text_buffer_get_highlight_tag (GspellInlineCheckerTextBuffer *spell)
{
	g_return_val_if_fail (GSPELL_IS_INLINE_CHECKER_TEXT_BUFFER (spell), NULL);

	return spell->highlight_tag;
}

/* ex:set ts=8 noet: */
