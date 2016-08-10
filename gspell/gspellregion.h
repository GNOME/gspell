/* Do not edit: this file is generated from https://git.gnome.org/browse/gtksourceview/plain/gtksourceview/gtksourceregion.h */

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * gspellregion.h - GtkTextMark-based region utility
 * This file is part of GtkSourceView
 *
 * Copyright (C) 2002 Gustavo Giráldez <gustavo.giraldez@gmx.net>
 * Copyright (C) 2016 Sébastien Wilmet <swilmet@gnome.org>
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
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef GSPELL_REGION_H
#define GSPELL_REGION_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GSPELL_TYPE_REGION (_gspell_region_get_type ())

G_GNUC_INTERNAL
G_DECLARE_DERIVABLE_TYPE (GspellRegion, _gspell_region,
			  GSPELL, REGION,
			  GObject)

struct _GspellRegionClass
{
	GObjectClass parent_class;

	/* Padding for future expansion */
	gpointer padding[8];
};

/*
 * GspellRegionIter:
 *
 * #GspellRegionIter is an opaque datatype; ignore all its fields.
 * Initialize the iter with _gspell_region_get_start_region_iter().
 *
 * Since: 3.22
 */
typedef struct _GspellRegionIter GspellRegionIter;
struct _GspellRegionIter
{
	/*< private >*/
	gpointer dummy1;
	guint32  dummy2;
	gpointer dummy3;
};

G_GNUC_INTERNAL
GspellRegion *	_gspell_region_new			(GtkTextBuffer *buffer);

G_GNUC_INTERNAL
GtkTextBuffer *		_gspell_region_get_buffer		(GspellRegion *region);

G_GNUC_INTERNAL
void			_gspell_region_add_subregion		(GspellRegion   *region,
								 const GtkTextIter *_start,
								 const GtkTextIter *_end);

G_GNUC_INTERNAL
void			_gspell_region_add_region		(GspellRegion *region,
								 GspellRegion *region_to_add);

G_GNUC_INTERNAL
void			_gspell_region_subtract_subregion	(GspellRegion   *region,
								 const GtkTextIter *_start,
								 const GtkTextIter *_end);

G_GNUC_INTERNAL
void			_gspell_region_subtract_region	(GspellRegion *region,
								 GspellRegion *region_to_subtract);

G_GNUC_INTERNAL
GspellRegion *	_gspell_region_intersect_subregion	(GspellRegion   *region,
								 const GtkTextIter *_start,
								 const GtkTextIter *_end);

G_GNUC_INTERNAL
GspellRegion *	_gspell_region_intersect_region	(GspellRegion *region1,
								 GspellRegion *region2);

G_GNUC_INTERNAL
gboolean		_gspell_region_is_empty		(GspellRegion *region);

G_GNUC_INTERNAL
gboolean		_gspell_region_get_bounds		(GspellRegion *region,
								 GtkTextIter     *start,
								 GtkTextIter     *end);

G_GNUC_INTERNAL
void			_gspell_region_get_start_region_iter	(GspellRegion     *region,
								 GspellRegionIter *iter);

G_GNUC_INTERNAL
gboolean		_gspell_region_iter_is_end		(GspellRegionIter *iter);

G_GNUC_INTERNAL
gboolean		_gspell_region_iter_next		(GspellRegionIter *iter);

G_GNUC_INTERNAL
gboolean		_gspell_region_iter_get_subregion	(GspellRegionIter *iter,
								 GtkTextIter         *start,
								 GtkTextIter         *end);

G_GNUC_INTERNAL
gchar *			_gspell_region_to_string		(GspellRegion *region);

G_END_DECLS

#endif /* GSPELL_REGION_H */
