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

#include <gtk/gtk.h>
#include "marker-iter.h"
#include "marker-list.h"

struct overlayaz_marker_iter
{
    GtkListStore *list;
    GtkTreeIter iter;
    overlayaz_marker_t *marker;
    gint id;
};


overlayaz_marker_iter_t*
overlayaz_marker_iter_new(GtkListStore              *ml,
                          const overlayaz_marker_t **m)
{
    overlayaz_marker_iter_t *mi;
    GtkTreeIter iter;

    if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(ml), &iter))
        return NULL;

    mi = g_malloc0(sizeof(overlayaz_marker_iter_t));
    mi->list = ml;
    mi->iter = iter;
    mi->marker = overlayaz_marker_list_get(ml, &iter);
    mi->id = 1;
    *m = mi->marker;
    return mi;
}

gboolean
overlayaz_marker_iter_next(overlayaz_marker_iter_t   *mi,
                           const overlayaz_marker_t **m)
{
    if (mi == NULL)
        return FALSE;

    if (!gtk_tree_model_iter_next(GTK_TREE_MODEL(mi->list), &mi->iter))
        return FALSE;

    mi->marker = overlayaz_marker_list_get(mi->list, &mi->iter);
    mi->id++;
    *m = mi->marker;
    return TRUE;
}

gint
overlayaz_marker_iter_get_id(overlayaz_marker_iter_t *mi)
{
    if (mi == NULL)
        return 0;
    return mi->id;
}

void
overlayaz_marker_iter_free(overlayaz_marker_iter_t *mi)
{
    if (mi)
        g_free(mi);
}
