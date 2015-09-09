/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * gtktextregion.h - GtkTextMark based region utility functions
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2002 Gustavo Gir�ldez <gustavo.giraldez@gmx.net>
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

#ifndef __GTK_TEXT_REGION_H__
#define __GTK_TEXT_REGION_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _GtkTextRegion		GtkTextRegion;
typedef struct _GtkTextRegionIterator	GtkTextRegionIterator;

struct _GtkTextRegionIterator {
	/* GtkTextRegionIterator is an opaque datatype; ignore all these fields.
	 * Initialize the iter with gtk_text_region_get_iterator
	 * function
	 */
	/*< private >*/
	gpointer dummy1;
	guint32  dummy2;
	gpointer dummy3;
};

G_GNUC_INTERNAL
GtkTextRegion *gtk_text_region_new                          (GtkTextBuffer *buffer);

G_GNUC_INTERNAL
void           gtk_text_region_destroy                      (GtkTextRegion *region);

G_GNUC_INTERNAL
GtkTextBuffer *gtk_text_region_get_buffer                   (GtkTextRegion *region);

G_GNUC_INTERNAL
void           gtk_text_region_add                          (GtkTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

G_GNUC_INTERNAL
void           gtk_text_region_subtract                     (GtkTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

G_GNUC_INTERNAL
gint           gtk_text_region_subregions                   (GtkTextRegion *region);

G_GNUC_INTERNAL
gboolean       gtk_text_region_nth_subregion                (GtkTextRegion *region,
							     guint          subregion,
							     GtkTextIter   *start,
							     GtkTextIter   *end);

G_GNUC_INTERNAL
GtkTextRegion *gtk_text_region_intersect                    (GtkTextRegion     *region,
							     const GtkTextIter *_start,
							     const GtkTextIter *_end);

G_GNUC_INTERNAL
void           gtk_text_region_get_iterator                 (GtkTextRegion         *region,
                                                             GtkTextRegionIterator *iter,
                                                             guint                  start);

G_GNUC_INTERNAL
gboolean       gtk_text_region_iterator_is_end              (GtkTextRegionIterator *iter);

/* Returns FALSE if iterator is the end iterator */
G_GNUC_INTERNAL
gboolean       gtk_text_region_iterator_next	            (GtkTextRegionIterator *iter);

G_GNUC_INTERNAL
gboolean       gtk_text_region_iterator_get_subregion       (GtkTextRegionIterator *iter,
							     GtkTextIter           *start,
							     GtkTextIter           *end);

G_GNUC_INTERNAL
void           gtk_text_region_debug_print                  (GtkTextRegion *region);

G_END_DECLS

#endif /* __GTK_TEXT_REGION_H__ */
