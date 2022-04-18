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

#ifndef OVERLAYAZ_MARKER_LIST_H_
#define OVERLAYAZ_MARKER_LIST_H_
#include "marker.h"

GtkListStore* overlayaz_marker_list_new();
void overlayaz_marker_list_free(GtkListStore*);
void overlayaz_marker_list_clear(GtkListStore*);
overlayaz_marker_t* overlayaz_marker_list_get(GtkListStore*, GtkTreeIter*);
void overlayaz_marker_list_add(GtkListStore*, overlayaz_marker_t*);
void overlayaz_marker_list_update(GtkListStore*, GtkTreeIter*);
void overlayaz_marker_list_remove(GtkListStore*, GtkTreeIter*);
gint overlayaz_marker_list_count(GtkListStore*);

#endif
