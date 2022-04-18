/*
 *  overlayaz – photo visibility analysis software
 *  Copyright (c) 2020-2022  Konrad Kosmatka
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#ifndef OVERLAYAZ_MARKER_ITER_H_
#define OVERLAYAZ_MARKER_ITER_H_
#include "marker.h"

typedef struct overlayaz_marker_iter overlayaz_marker_iter_t;

overlayaz_marker_iter_t* overlayaz_marker_iter_new(GtkListStore*, const overlayaz_marker_t**);
gboolean overlayaz_marker_iter_next(overlayaz_marker_iter_t*, const overlayaz_marker_t**);
gint overlayaz_marker_iter_get_id(overlayaz_marker_iter_t*);
void overlayaz_marker_iter_free(overlayaz_marker_iter_t*);

#endif
