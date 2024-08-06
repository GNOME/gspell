/* SPDX-FileCopyrightText: 2002 - Paolo Maggi
 * SPDX-FileCopyrightText: 2015, 2016, 2017 - Sébastien Wilmet
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* This is a modified version of GtkSpell 2.0.5 (gtkspell.sf.net)
 * SPDX-FileCopyrightText: 2002 - Evan Martin
 */

#include "gspell-config.h"
#include "gspell-inline-checker-text-buffer.h"
#include <string.h>
#include <glib/gi18n-lib.h>
#include "gspellregion.h"
#include "gspell-checker.h"
#include "gspell-context-menu.h"
#include "gspell-current-word-policy.h"
#include "gspell-text-buffer.h"
#include "gspell-text-iter.h"
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

	GspellRegion *scan_region;
	guint timeout_id;

	GspellCurrentWordPolicy *current_word_policy;

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

typedef enum
{
	ADJUST_MODE_STRICTLY_INSIDE_WORD,
	ADJUST_MODE_INCLUDE_NEIGHBORS,
} AdjustMode;

#define INLINE_CHECKER_TEXT_BUFFER_KEY "GspellInlineCheckerTextBufferID"

/* Timeout durations in milliseconds. Writing and deleting text should be smooth
 * and responsive.
 */
#define TIMEOUT_DURATION_BUFFER_MODIFIED 16
#define TIMEOUT_DURATION_DRAWING 20

#define PERF_DEBUG FALSE

G_DEFINE_TYPE (GspellInlineCheckerTextBuffer, _gspell_inline_checker_text_buffer, G_TYPE_OBJECT)

/* Remove the highlight_tag only if present. If gtk_text_buffer_remove_tag() is
 * called when the tag is not present, GtkTextView anyway queues a redraw, which
 * we want to avoid (it can lead to an infinite loop).
 */
static void
remove_highlight_tag_if_present (GspellInlineCheckerTextBuffer *spell,
				 const GtkTextIter             *start,
				 const GtkTextIter             *end)
{
	gboolean remove = FALSE;

	if (gtk_text_iter_has_tag (start, spell->highlight_tag))
	{
		remove = TRUE;
	}
	else
	{
		GtkTextIter iter = *start;

		if (gtk_text_iter_forward_to_tag_toggle (&iter, spell->highlight_tag) &&
		    gtk_text_iter_compare (&iter, end) < 0)
		{
			remove = TRUE;
		}
	}

	if (remove)
	{
		gtk_text_buffer_remove_tag (spell->buffer,
					    spell->highlight_tag,
					    start,
					    end);
	}
}

static void
adjust_iters (GtkTextIter *start,
	      GtkTextIter *end,
	      AdjustMode   mode)
{
	switch (mode)
	{
		case ADJUST_MODE_STRICTLY_INSIDE_WORD:
			if (_gspell_text_iter_inside_word (start) &&
			    !_gspell_text_iter_starts_word (start))
			{
				_gspell_text_iter_backward_word_start (start);
			}

			if (_gspell_text_iter_inside_word (end) &&
			    !_gspell_text_iter_starts_word (end))
			{
				_gspell_text_iter_forward_word_end (end);
			}
			break;

		case ADJUST_MODE_INCLUDE_NEIGHBORS:
			if (_gspell_text_iter_ends_word (start) ||
			    (_gspell_text_iter_inside_word (start) &&
			     !_gspell_text_iter_starts_word (start)))
			{
				_gspell_text_iter_backward_word_start (start);
			}

			if (_gspell_text_iter_inside_word (end))
			{
				_gspell_text_iter_forward_word_end (end);
			}
			break;

		default:
			g_assert_not_reached ();
	}
}

/* Free *attrs with g_free() when no longer needed. */
static void
get_pango_log_attrs (const gchar   *text,
		     PangoLogAttr **attrs,
		     gint          *n_attrs)
{
	*n_attrs = g_utf8_strlen (text, -1) + 1;
	*attrs = g_new0 (PangoLogAttr, *n_attrs);

	pango_get_log_attrs (text,
			     strlen (text),
			     -1,
			     NULL,
			     *attrs,
			     *n_attrs);

	_gspell_utils_improve_word_boundaries (text, *attrs, *n_attrs);
}

static gboolean
should_apply_tag_to_misspelled_word (GspellInlineCheckerTextBuffer *spell,
				     const GtkTextIter             *word_start,
				     const GtkTextIter             *word_end)
{
	GtkTextIter iter;

	if (spell->no_spell_check_tag == NULL)
	{
		return TRUE;
	}

	if (gtk_text_iter_has_tag (word_start, spell->no_spell_check_tag))
	{
		return FALSE;
	}

	iter = *word_start;
	if (!gtk_text_iter_forward_to_tag_toggle (&iter, spell->no_spell_check_tag))
	{
		return TRUE;
	}

	return gtk_text_iter_compare (word_end, &iter) <= 0;
}

/* A first implementation of this function used the _gspell_text_iter_*()
 * functions in a loop to navigate through words between @start and @end.
 * But the _gspell_text_iter_*() functions are *slow*. So a new implementation
 * has been written to reduce the number of calls to GtkTextView functions, and
 * it's up to 20x faster! (200 ms -> 10 ms).
 * And there is most probably still room for performance improvements.
 */
static void
check_subregion (GspellInlineCheckerTextBuffer *spell,
		 GtkTextIter                   *start,
		 GtkTextIter                   *end)
{
	gchar *text;
	const gchar *cur_text_pos;
	const gchar *word_start;
	gint word_start_char_pos;
	PangoLogAttr *attrs;
	gint n_attrs;
	gint attr_num;
	gint start_offset;

	g_return_if_fail (gtk_text_iter_compare (start, end) <= 0);

	adjust_iters (start, end, ADJUST_MODE_STRICTLY_INSIDE_WORD);

	gtk_text_buffer_remove_tag (spell->buffer,
				    spell->highlight_tag,
				    start,
				    end);

	if (spell->spell_checker == NULL ||
	    gspell_checker_get_language (spell->spell_checker) == NULL)
	{
		return;
	}

	text = gtk_text_iter_get_slice (start, end);

	if (text == NULL || text[0] == '\0')
	{
		g_free (text);
		return;
	}

	get_pango_log_attrs (text, &attrs, &n_attrs);

	attr_num = 0;
	cur_text_pos = text;
	word_start = NULL;
	word_start_char_pos = 0;

	start_offset = gtk_text_iter_get_offset (start);

	while (attr_num < n_attrs)
	{
		PangoLogAttr *cur_attr = &attrs[attr_num];

		if (word_start != NULL &&
		    cur_attr->is_word_end)
		{
			gint word_byte_length;
			gboolean misspelled;
			GError *error = NULL;

			if (cur_text_pos != NULL)
			{
				word_byte_length = cur_text_pos - word_start;
			}
			else
			{
				word_byte_length = -1;
			}

			misspelled = !gspell_checker_check_word (spell->spell_checker,
								 word_start,
								 word_byte_length,
								 &error);

			if (error != NULL)
			{
				g_warning ("Inline spell checker: %s", error->message);
				g_clear_error (&error);
			}

			if (misspelled)
			{
				gint word_start_offset;
				gint word_end_offset;
				GtkTextIter word_start_iter;
				GtkTextIter word_end_iter;

				word_start_offset = start_offset + word_start_char_pos;
				word_end_offset = start_offset + attr_num;

				gtk_text_buffer_get_iter_at_offset (spell->buffer,
								    &word_start_iter,
								    word_start_offset);

				gtk_text_buffer_get_iter_at_offset (spell->buffer,
								    &word_end_iter,
								    word_end_offset);

				/* FIXME: it's a bit stupid to spell-check words
				 * in the no-spell-check region. The relevant
				 * word boundaries in the PangoLogAttr array
				 * should be removed beforehand.
				 */
				if (should_apply_tag_to_misspelled_word (spell, &word_start_iter, &word_end_iter))
				{
					gtk_text_buffer_apply_tag (spell->buffer,
								   spell->highlight_tag,
								   &word_start_iter,
								   &word_end_iter);
				}
			}

			/* Find next word start. */
			word_start = NULL;
		}

		if (word_start == NULL &&
		    cur_attr->is_word_start)
		{
			word_start = cur_text_pos;
			word_start_char_pos = attr_num;
		}

		if (attr_num == n_attrs - 1 ||
		    cur_text_pos == NULL ||
		    cur_text_pos[0] == '\0')
		{
			break;
		}

		attr_num++;
		cur_text_pos = g_utf8_find_next_char (cur_text_pos, NULL);
	}

	/* Sanity checks */

	if (attr_num != n_attrs - 1)
	{
		g_warning ("%s(): problem in loop iteration, attr_num=%d but should be %d. "
			   "End of string reached too early.",
			   G_STRFUNC,
			   attr_num,
			   n_attrs - 1);
	}

	if (cur_text_pos != NULL && cur_text_pos[0] != '\0')
	{
		g_warning ("%s(): end of string not reached.", G_STRFUNC);
	}

	g_free (text);
	g_free (attrs);
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

/* Returns TRUE if there is a current word. */
static gboolean
get_current_word_boundaries (GtkTextBuffer *buffer,
			     GtkTextIter   *current_word_start,
			     GtkTextIter   *current_word_end)
{
	GtkTextIter insert_iter;

	g_assert (current_word_start != NULL);
	g_assert (current_word_end != NULL);

	if (gtk_text_buffer_get_has_selection (buffer))
	{
		return FALSE;
	}

	gtk_text_buffer_get_iter_at_mark (buffer,
					  &insert_iter,
					  gtk_text_buffer_get_insert (buffer));

	*current_word_start = insert_iter;
	*current_word_end = insert_iter;

	adjust_iters (current_word_start, current_word_end, ADJUST_MODE_INCLUDE_NEIGHBORS);

	return (!gtk_text_iter_equal (current_word_start, &insert_iter) ||
		!gtk_text_iter_equal (current_word_end, &insert_iter));
}

static void
check_visible_region_in_view (GspellInlineCheckerTextBuffer *spell,
			      GtkTextView                   *view)
{
	GtkTextIter visible_start;
	GtkTextIter visible_end;
	GspellRegion *intersect;
	GspellRegionIter intersect_iter;

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
		g_assert (spell->unit_test_mode);
		gtk_text_buffer_get_bounds (spell->buffer, &visible_start, &visible_end);
	}

	intersect = _gspell_region_intersect_subregion (spell->scan_region,
							&visible_start,
							&visible_end);

	if (_gspell_region_is_empty (intersect))
	{
		g_clear_object (&intersect);
		return;
	}

	if (!_gspell_current_word_policy_get_check_current_word (spell->current_word_policy))
	{
		GtkTextIter current_word_start;
		GtkTextIter current_word_end;

		if (get_current_word_boundaries (spell->buffer,
						 &current_word_start,
						 &current_word_end))
		{
			remove_highlight_tag_if_present (spell,
							 &current_word_start,
							 &current_word_end);

			_gspell_region_subtract_subregion (intersect,
							   &current_word_start,
							   &current_word_end);

			/* Be sure that the current word will be re-checked
			 * later when it will no longer be the current word.
			 */
			_gspell_region_add_subregion (spell->scan_region,
						      &current_word_start,
						      &current_word_end);

			if (_gspell_region_is_empty (intersect))
			{
				g_clear_object (&intersect);
				return;
			}
		}
	}

	_gspell_region_get_start_region_iter (intersect, &intersect_iter);

	while (!_gspell_region_iter_is_end (&intersect_iter))
	{
		GtkTextIter start;
		GtkTextIter end;
		GtkTextIter orig_start;
		GtkTextIter orig_end;
		gboolean bug = FALSE;

		if (!_gspell_region_iter_get_subregion (&intersect_iter, &start, &end))
		{
			break;
		}

		orig_start = start;
		orig_end = end;

		{
#if PERF_DEBUG
			GTimer *timer;

			g_print ("check_subregion [%d, %d]\n",
				 gtk_text_iter_get_offset (&start),
				 gtk_text_iter_get_offset (&end));

			timer = g_timer_new ();
#endif

			check_subregion (spell, &start, &end);

#if PERF_DEBUG
			g_print ("check_subregion took %lf ms.\n\n",
				 1000 * g_timer_elapsed (timer, NULL));
			g_timer_destroy (timer);
#endif
		}

		/* Ensure that we don't have an infinite loop. We must subtract
		 * from scan_region at least [orig_start, orig_end], otherwise
		 * we will re-check the same subregion again and again.
		 */
		if (gtk_text_iter_compare (&orig_start, &start) < 0)
		{
			g_warning ("Should not reach this code path.");
			bug = TRUE;
			start = orig_start;
		}
		if (gtk_text_iter_compare (&end, &orig_end) < 0)
		{
			g_warning ("Should not reach this code path.");
			bug = TRUE;
			end = orig_end;
		}

		if (bug)
		{
			gchar *text;

			text = gtk_text_iter_get_slice (&start, &end);
			g_warning ("Text that caused the bug: '%s'", text);
			g_warning ("Please report the bug to: " PACKAGE_BUGREPORT);
			g_free (text);
		}

		_gspell_region_subtract_subregion (spell->scan_region, &start, &end);

		_gspell_region_iter_next (&intersect_iter);
	}

	g_clear_object (&intersect);

	if (_gspell_region_is_empty (spell->scan_region))
	{
		g_clear_object (&spell->scan_region);
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
		spell->scan_region = _gspell_region_new (spell->buffer);
	}

	_gspell_region_add_subregion (spell->scan_region, start, end);
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

/* The word boundaries are not necessarily the same before and after a text
 * insertion or deletion. We need the broader boundaries, so we connect to the
 * signal without and with the AFTER flag.
 */
static void
insert_text_before_cb (GtkTextBuffer                 *buffer,
		       GtkTextIter                   *location,
		       gchar                         *text,
		       gint                           length,
		       GspellInlineCheckerTextBuffer *spell)
{
	GtkTextIter start;
	GtkTextIter end;

	start = *location;
	end = *location;
	adjust_iters (&start, &end, ADJUST_MODE_INCLUDE_NEIGHBORS);
	add_subregion_to_scan (spell, &start, &end);

	/* Don't install_timeout(), it will anyway be called in
	 * insert_text_after_cb(). If install_timeout() is called here, it is a
	 * problem for the unit test mode because the subregion would be scanned
	 * directly, but we need to wait that the text is inserted, otherwise
	 * this can give different results (since the word boundaries are not
	 * necessarily the same).
	 */
}

static void
insert_text_after_cb (GtkTextBuffer                 *buffer,
		      GtkTextIter                   *location,
		      gchar                         *text,
		      gint                           length,
		      GspellInlineCheckerTextBuffer *spell)
{
	glong n_chars;
	GtkTextIter start;
	GtkTextIter end;

	n_chars = g_utf8_strlen (text, length);

	start = *location;
	end = *location;
	gtk_text_iter_backward_chars (&start, n_chars);

	adjust_iters (&start, &end, ADJUST_MODE_INCLUDE_NEIGHBORS);
	add_subregion_to_scan (spell, &start, &end);

	/* Check current word? */
	if (n_chars > 1)
	{
		_gspell_current_word_policy_several_chars_inserted (spell->current_word_policy);
	}
	else
	{
		gunichar ch;
		gboolean empty_selection;
		GtkTextIter cursor_pos;
		gboolean at_cursor_pos;

		ch = g_utf8_get_char (text);
		empty_selection = !gtk_text_buffer_get_has_selection (buffer);

		gtk_text_buffer_get_iter_at_mark (buffer,
						  &cursor_pos,
						  gtk_text_buffer_get_insert (buffer));

		at_cursor_pos = gtk_text_iter_equal (location, &cursor_pos);

		_gspell_current_word_policy_single_char_inserted (spell->current_word_policy,
								  ch,
								  empty_selection,
								  at_cursor_pos);
	}

	install_timeout (spell, TIMEOUT_DURATION_BUFFER_MODIFIED);
}

/* Same reasoning as for the ::insert-text signal. */
static void
delete_range_before_cb (GtkTextBuffer                 *buffer,
			GtkTextIter                   *start,
			GtkTextIter                   *end,
			GspellInlineCheckerTextBuffer *spell)
{
	{
		GtkTextIter start_adjusted;
		GtkTextIter end_adjusted;

		start_adjusted = *start;
		end_adjusted = *end;
		adjust_iters (&start_adjusted, &end_adjusted, ADJUST_MODE_INCLUDE_NEIGHBORS);
		add_subregion_to_scan (spell, &start_adjusted, &end_adjusted);
	}

	/* Check current word? */
	{
		gboolean empty_selection;
		gboolean spans_several_lines;
		gboolean several_chars;
		GtkTextIter cursor_pos;
		gboolean cursor_pos_at_start;
		gboolean cursor_pos_at_end;
		gboolean start_is_inside_word;
		gboolean start_ends_word;
		gboolean end_is_inside_word;
		gboolean end_ends_word;

		empty_selection = !gtk_text_buffer_get_has_selection (buffer);
		spans_several_lines = gtk_text_iter_get_line (start) != gtk_text_iter_get_line (end);
		several_chars = gtk_text_iter_get_offset (end) - gtk_text_iter_get_offset (start) > 1;

		gtk_text_buffer_get_iter_at_mark (buffer,
						  &cursor_pos,
						  gtk_text_buffer_get_insert (buffer));

		cursor_pos_at_start = gtk_text_iter_equal (&cursor_pos, start);
		cursor_pos_at_end = gtk_text_iter_equal (&cursor_pos, end);

		start_is_inside_word = _gspell_text_iter_inside_word (start);
		start_ends_word = _gspell_text_iter_ends_word (start);
		end_is_inside_word = _gspell_text_iter_inside_word (end);
		end_ends_word = _gspell_text_iter_ends_word (end);

		_gspell_current_word_policy_text_deleted (spell->current_word_policy,
							  empty_selection,
							  spans_several_lines,
							  several_chars,
							  cursor_pos_at_start,
							  cursor_pos_at_end,
							  start_is_inside_word,
							  start_ends_word,
							  end_is_inside_word,
							  end_ends_word);
	}
}

static void
delete_range_after_cb (GtkTextBuffer                 *buffer,
		       GtkTextIter                   *start,
		       GtkTextIter                   *end,
		       GspellInlineCheckerTextBuffer *spell)
{
	GtkTextIter start_adjusted;
	GtkTextIter end_adjusted;

	g_return_if_fail (gtk_text_iter_equal (start, end));

	start_adjusted = *start;
	end_adjusted = *end;
	adjust_iters (&start_adjusted, &end_adjusted, ADJUST_MODE_INCLUDE_NEIGHBORS);
	add_subregion_to_scan (spell, &start_adjusted, &end_adjusted);

	install_timeout (spell, TIMEOUT_DURATION_BUFFER_MODIFIED);
}

static void
mark_set_after_cb (GtkTextBuffer                 *buffer,
		   GtkTextIter                   *location,
		   GtkTextMark                   *mark,
		   GspellInlineCheckerTextBuffer *spell)
{
	if (mark == gtk_text_buffer_get_insert (buffer))
	{
		_gspell_current_word_policy_cursor_moved (spell->current_word_policy);
		install_timeout (spell, TIMEOUT_DURATION_BUFFER_MODIFIED);
	}
}

static gboolean
get_word_extents_at_click_position (GspellInlineCheckerTextBuffer *spell,
				    GtkTextIter                   *start,
				    GtkTextIter                   *end)
{
	GtkTextIter iter;

	gtk_text_buffer_get_iter_at_mark (spell->buffer, &iter, spell->mark_click);

	if (!_gspell_text_iter_inside_word (&iter) &&
	    !_gspell_text_iter_ends_word (&iter))
	{
		return FALSE;
	}

	*start = iter;
	if (!_gspell_text_iter_starts_word (start))
	{
		_gspell_text_iter_backward_word_start (start);
	}

	*end = iter;
	if (!_gspell_text_iter_ends_word (end))
	{
		_gspell_text_iter_forward_word_end (end);
	}

	return TRUE;
}

static void
suggestion_activated_cb (const gchar *suggested_word,
			 gpointer     user_data)
{
	GspellInlineCheckerTextBuffer *spell;
	GtkTextIter start;
	GtkTextIter end;
	gchar *old_word;

	g_return_if_fail (GSPELL_IS_INLINE_CHECKER_TEXT_BUFFER (user_data));

	spell = GSPELL_INLINE_CHECKER_TEXT_BUFFER (user_data);

	if (!get_word_extents_at_click_position (spell, &start, &end))
	{
		return;
	}

	old_word = gtk_text_buffer_get_text (spell->buffer, &start, &end, FALSE);

	gtk_text_buffer_begin_user_action (spell->buffer);

	gtk_text_buffer_delete (spell->buffer, &start, &end);
	gtk_text_buffer_insert (spell->buffer, &start, suggested_word, -1);

	gtk_text_buffer_end_user_action (spell->buffer);

	if (spell->spell_checker != NULL)
	{
		gspell_checker_set_correction (spell->spell_checker,
					       old_word, -1,
					       suggested_word, -1);
	}

	g_free (old_word);
}

void
_gspell_inline_checker_text_buffer_populate_popup (GspellInlineCheckerTextBuffer *spell,
						   GtkMenu                       *menu)
{
	GtkMenuItem *menu_item;
	GtkTextIter start;
	GtkTextIter end;
	gchar *misspelled_word;

	if (!get_word_extents_at_click_position (spell, &start, &end))
	{
		return;
	}

	if (!gtk_text_iter_has_tag (&start, spell->highlight_tag))
	{
		return;
	}

	if (spell->spell_checker == NULL)
	{
		return;
	}

	/* Prepend suggestions */

	misspelled_word = gtk_text_buffer_get_text (spell->buffer, &start, &end, FALSE);

	menu_item = _gspell_context_menu_get_suggestions_menu_item (spell->spell_checker,
								    misspelled_word,
								    suggestion_activated_cb,
								    spell);

	gtk_menu_shell_prepend (GTK_MENU_SHELL (menu),
				GTK_WIDGET (menu_item));

	g_free (misspelled_word);
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

		if (_gspell_text_iter_starts_word (&match_start) &&
		    _gspell_text_iter_ends_word (&match_end))
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
	_gspell_current_word_policy_session_cleared (spell->current_word_policy);
	recheck_all (spell);
}

static void
language_notify_cb (GspellChecker                 *checker,
		    GParamSpec                    *pspec,
		    GspellInlineCheckerTextBuffer *spell)
{
	_gspell_current_word_policy_language_changed (spell->current_word_policy);
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

		_gspell_current_word_policy_set_check_current_word (spell->current_word_policy, TRUE);
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

		_gspell_current_word_policy_set_check_current_word (spell->current_word_policy, TRUE);
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
spell_checker_notify_cb (GspellTextBuffer              *gspell_buffer,
			 GParamSpec                    *pspec,
			 GspellInlineCheckerTextBuffer *spell)
{
	GspellChecker *new_checker;

	new_checker = gspell_text_buffer_get_spell_checker (gspell_buffer);
	set_spell_checker (spell, new_checker);

	_gspell_current_word_policy_checker_changed (spell->current_word_policy);
	recheck_all (spell);
}

static void
set_buffer (GspellInlineCheckerTextBuffer *spell,
	    GtkTextBuffer                 *buffer)
{
	GtkTextTagTable *tag_table;
	GtkTextIter start;
	GspellTextBuffer *gspell_buffer;
	GspellChecker *checker;
	GdkRGBA underline_color;

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
				 G_CALLBACK (insert_text_before_cb),
				 spell,
				 0);

	g_signal_connect_object (buffer,
				 "insert-text",
				 G_CALLBACK (insert_text_after_cb),
				 spell,
				 G_CONNECT_AFTER);

	g_signal_connect_object (buffer,
				 "delete-range",
				 G_CALLBACK (delete_range_before_cb),
				 spell,
				 0);

	g_signal_connect_object (buffer,
				 "delete-range",
				 G_CALLBACK (delete_range_after_cb),
				 spell,
				 G_CONNECT_AFTER);

	g_signal_connect_object (buffer,
				 "mark-set",
				 G_CALLBACK (mark_set_after_cb),
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

	_gspell_utils_init_underline_rgba (&underline_color);

	spell->highlight_tag = gtk_text_buffer_create_tag (spell->buffer, NULL,
							   "underline", PANGO_UNDERLINE_SINGLE,
							   "underline-rgba", &underline_color,
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

	gspell_buffer = gspell_text_buffer_get_from_gtk_text_buffer (spell->buffer);
	checker = gspell_text_buffer_get_spell_checker (gspell_buffer);
	set_spell_checker (spell, checker);

	g_signal_connect_object (gspell_buffer,
				 "notify::spell-checker",
				 G_CALLBACK (spell_checker_notify_cb),
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
	g_clear_object (&spell->scan_region);
	g_clear_object (&spell->current_word_policy);

	g_slist_free (spell->views);
	spell->views = NULL;

	spell->mark_click = NULL;

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
	spell->current_word_policy = _gspell_current_word_policy_new ();
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
				 "draw",
				 G_CALLBACK (draw_cb),
				 spell,
				 0);

	spell->views = g_slist_prepend (spell->views, view);

	_gspell_current_word_policy_set_check_current_word (spell->current_word_policy, TRUE);
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
