/* Do not edit: this file is generated from https://git.gnome.org/browse/gtksourceview/plain/gtksourceview/gtktextregion.h */

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * gspell-text-region.h - GtkTextMark based region utility functions
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2002 Gustavo Giráldez <gustavo.giraldez@gmx.net>
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

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __GSPELL_TEXT_REGION_H__
#define __GSPELL_TEXT_REGION_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _GspellTextRegion		GspellTextRegion;
typedef struct _GspellTextRegionIterator	GspellTextRegionIterator;

struct _GspellTextRegionIterator {
	/* GspellTextRegionIterator is an opaque datatype; ignore all these fields.
	 * Initialize the iter with gspell_text_region_get_iterator
	 * function
	 */
	/*< private >*/
	gpointer dummy1;
	guint32  dummy2;
	gpointer dummy3;
};

G_GNUC_INTERNAL
GspellTextRegion *gspell_text_region_new                          (GtkTextBuffer *buffer);

G_GNUC_INTERNAL
void           gspell_text_region_destroy                      (GspellTextRegion *region);

G_GNUC_INTERNAL
GtkTextBuffer *gspell_text_region_get_buffer                   (GspellTextRegion *region);

G_GNUC_INTERNAL
void           gspell_text_region_add                          (GspellTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

G_GNUC_INTERNAL
void           gspell_text_region_subtract                     (GspellTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

G_GNUC_INTERNAL
gint           gspell_text_region_subregions                   (GspellTextRegion *region);

G_GNUC_INTERNAL
gboolean       gspell_text_region_nth_subregion                (GspellTextRegion *region,
							     guint          subregion,
							     GtkTextIter   *start,
							     GtkTextIter   *end);

G_GNUC_INTERNAL
GspellTextRegion *gspell_text_region_intersect                    (GspellTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

G_GNUC_INTERNAL
void           gspell_text_region_get_iterator                 (GspellTextRegion         *region,
                                                             GspellTextRegionIterator *iter,
                                                             guint                  start);

G_GNUC_INTERNAL
gboolean       gspell_text_region_iterator_is_end              (GspellTextRegionIterator *iter);

/* Returns FALSE if iterator is the end iterator */
G_GNUC_INTERNAL
gboolean       gspell_text_region_iterator_next	            (GspellTextRegionIterator *iter);

G_GNUC_INTERNAL
gboolean       gspell_text_region_iterator_get_subregion       (GspellTextRegionIterator *iter,
							     GtkTextIter           *start,
							     GtkTextIter           *end);

G_GNUC_INTERNAL
void           gspell_text_region_debug_print                  (GspellTextRegion *region);

G_END_DECLS

#endif /* __GSPELL_TEXT_REGION_H__ */
