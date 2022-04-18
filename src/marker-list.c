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
#include "marker-list.h"

static gboolean marker_list_foreach_free(GtkTreeModel*, GtkTreePath*, GtkTreeIter*, gpointer);

/* No need to call overlayaz_changed() anywhere,
 * GtkListStore will emit signals on its own. */

GtkListStore*
overlayaz_marker_list_new()
{
    return gtk_list_store_new(1, G_TYPE_POINTER);
}

void
overlayaz_marker_list_free(GtkListStore *ml)
{
    if (ml)
    {
        overlayaz_marker_list_clear(ml);
        g_object_unref(ml);
    }
}

void
overlayaz_marker_list_clear(GtkListStore *ml)
{
    gtk_tree_model_foreach(GTK_TREE_MODEL(ml), marker_list_foreach_free, NULL);
    gtk_list_store_clear(ml);
}

overlayaz_marker_t*
overlayaz_marker_list_get(GtkListStore *ml,
                          GtkTreeIter  *iter)
{
    overlayaz_marker_t *marker;
    gtk_tree_model_get(GTK_TREE_MODEL(ml), iter, 0, &marker, -1);
    return marker;
}

void
overlayaz_marker_list_add(GtkListStore       *ml,
                          overlayaz_marker_t *m)
{
    GtkTreeIter iter;
    gtk_list_store_append(ml, &iter);
    gtk_list_store_set(ml, &iter, 0, m, -1);
}

void
overlayaz_marker_list_update(GtkListStore *ml,
                             GtkTreeIter  *iter)
{
    GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(ml), iter);
    gtk_tree_model_row_changed(GTK_TREE_MODEL(ml), path, iter);
    gtk_tree_path_free(path);
}

void
overlayaz_marker_list_remove(GtkListStore *ml,
                             GtkTreeIter  *iter)
{
    overlayaz_marker_t *marker;
    gtk_tree_model_get(GTK_TREE_MODEL(ml), iter, 0, &marker, -1);
    overlayaz_marker_free(marker);
    gtk_list_store_remove(ml, iter);
}

gint
overlayaz_marker_list_count(GtkListStore *ml)
{
    return gtk_tree_model_iter_n_children(GTK_TREE_MODEL(ml), NULL);
}

static gboolean
marker_list_foreach_free(GtkTreeModel *ml,
                         GtkTreePath  *path,
                         GtkTreeIter  *iter,
                         gpointer      data)
{
    overlayaz_marker_t *m;
    gtk_tree_model_get(ml, iter, 0, &m, -1);
    overlayaz_marker_free(m);
    return FALSE;
}
