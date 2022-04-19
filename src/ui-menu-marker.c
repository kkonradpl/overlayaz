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
#include "ui.h"
#include "ui-menu-marker.h"
#include "geo.h"
#include "marker-list.h"
#include "dialog.h"

struct overlayaz_ui_menu_marker
{
    overlayaz_ui_t *ui;
    struct overlayaz_menu_marker *m;
    overlayaz_t *o;
    gboolean lock;
};

static void ui_menu_marker_format_name(GtkCellLayout*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);
static void ui_menu_marker_combo_changed(GtkComboBox*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_button_down(GtkButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_button_up(GtkButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_button_remove(GtkButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_button_clear(GtkButton*, overlayaz_ui_menu_marker_t*);

static gboolean ui_menu_marker_textview_name_key_press(GtkWidget*, GdkEventKey*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_textbuffer_name_changed(GtkTextBuffer*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_spin_lat_changed(GtkSpinButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_spin_lon_changed(GtkSpinButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_combo_changed_tick(GtkComboBox*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_font_set(GtkFontButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_font_color_set(GtkColorButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_scale_pos_changed(GtkRange*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_check_active_toggled(GtkToggleButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_check_show_azi_toggled(GtkToggleButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_check_show_dist_toggled(GtkToggleButton*, overlayaz_ui_menu_marker_t*);

static void ui_menu_marker_spin_latlon_changed(GtkSpinButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_spin_azidist_changed(GtkSpinButton*, overlayaz_ui_menu_marker_t*);

static void ui_menu_marker_button_tick_apply(GtkButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_button_font_apply(GtkButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_button_pos_apply(GtkButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_button_active_apply(GtkButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_button_show_azi_apply(GtkButton*, overlayaz_ui_menu_marker_t*);
static void ui_menu_marker_button_show_dist_apply(GtkButton*, overlayaz_ui_menu_marker_t*);

static gboolean ui_marker_dialog_apply(overlayaz_ui_t*, const gchar*);


overlayaz_ui_menu_marker_t*
overlayaz_ui_menu_marker_new(overlayaz_ui_t               *ui,
                             struct overlayaz_menu_marker *m,
                             overlayaz_t                  *o)
{
    overlayaz_ui_menu_marker_t *ui_m = g_malloc0(sizeof(overlayaz_ui_menu_marker_t));
    GtkTextBuffer *text_buffer;

    ui_m->ui = ui;
    ui_m->m = m;
    ui_m->o = o;

    gtk_combo_box_set_model(GTK_COMBO_BOX(ui_m->m->combo_marker),
                            GTK_TREE_MODEL(overlayaz_get_marker_list(o)));
    gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(ui_m->m->combo_marker),
                                       ui_m->m->renderer_marker,
                                       ui_menu_marker_format_name,
                                       NULL, NULL);
    g_signal_connect(ui_m->m->combo_marker, "changed", G_CALLBACK(ui_menu_marker_combo_changed), ui_m);

    g_signal_connect(ui_m->m->button_down, "clicked", G_CALLBACK(ui_menu_marker_button_down), ui_m);
    g_signal_connect(ui_m->m->button_up, "clicked", G_CALLBACK(ui_menu_marker_button_up), ui_m);
    g_signal_connect(ui_m->m->button_remove, "clicked", G_CALLBACK(ui_menu_marker_button_remove), ui_m);
    g_signal_connect(ui_m->m->button_clear, "clicked", G_CALLBACK(ui_menu_marker_button_clear), ui_m);

    g_signal_connect_after(ui_m->m->textview_name, "key-press-event", G_CALLBACK(ui_menu_marker_textview_name_key_press), ui);
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ui_m->m->textview_name));
    g_signal_connect(text_buffer, "changed", G_CALLBACK(ui_menu_marker_textbuffer_name_changed), ui_m);
    g_signal_connect(ui_m->m->spin_lat, "changed", G_CALLBACK(ui_menu_marker_spin_lat_changed), ui_m);
    g_signal_connect(ui_m->m->spin_lat, "changed", G_CALLBACK(ui_menu_marker_spin_latlon_changed), ui_m);
    g_signal_connect(ui_m->m->spin_lon, "changed", G_CALLBACK(ui_menu_marker_spin_lon_changed), ui_m);
    g_signal_connect(ui_m->m->spin_lon, "changed", G_CALLBACK(ui_menu_marker_spin_latlon_changed), ui_m);
    g_signal_connect(ui_m->m->spin_azi, "changed", G_CALLBACK(ui_menu_marker_spin_azidist_changed), ui_m);
    g_signal_connect(ui_m->m->spin_dist, "changed", G_CALLBACK(ui_menu_marker_spin_azidist_changed), ui_m);
    g_signal_connect(ui_m->m->combo_tick, "changed", G_CALLBACK(ui_menu_marker_combo_changed_tick), ui_m);
    g_signal_connect(ui_m->m->font_marker, "font-set", G_CALLBACK(ui_menu_marker_font_set), ui_m);
    g_signal_connect(ui_m->m->color_marker_font, "color-set", G_CALLBACK(ui_menu_marker_font_color_set), ui_m);
    g_signal_connect(ui_m->m->scale_pos, "value-changed", G_CALLBACK(ui_menu_marker_scale_pos_changed), ui_m);
    g_signal_connect(ui_m->m->check_active, "toggled", G_CALLBACK(ui_menu_marker_check_active_toggled), ui_m);
    g_signal_connect(ui_m->m->check_show_azi, "toggled", G_CALLBACK(ui_menu_marker_check_show_azi_toggled), ui_m);
    g_signal_connect(ui_m->m->check_show_dist, "toggled", G_CALLBACK(ui_menu_marker_check_show_dist_toggled), ui_m);

    g_signal_connect(ui_m->m->button_tick_apply, "clicked", G_CALLBACK(ui_menu_marker_button_tick_apply), ui_m);
    g_signal_connect(ui_m->m->button_font_apply, "clicked", G_CALLBACK(ui_menu_marker_button_font_apply), ui_m);
    g_signal_connect(ui_m->m->button_pos_apply, "clicked", G_CALLBACK(ui_menu_marker_button_pos_apply), ui_m);
    g_signal_connect(ui_m->m->button_active_apply, "clicked", G_CALLBACK(ui_menu_marker_button_active_apply), ui_m);
    g_signal_connect(ui_m->m->button_show_azi_apply, "clicked", G_CALLBACK(ui_menu_marker_button_show_azi_apply), ui_m);
    g_signal_connect(ui_m->m->button_show_dist_apply, "clicked", G_CALLBACK(ui_menu_marker_button_show_dist_apply), ui_m);

    return ui_m;
}

void
overlayaz_ui_menu_marker_free(overlayaz_ui_menu_marker_t *ui_m)
{
    g_free(ui_m);
}

void
overlayaz_ui_menu_marker_sync(overlayaz_ui_menu_marker_t *ui_m,
                              gboolean                    active)
{
    GtkTreeModel *model;

    gtk_widget_set_sensitive(ui_m->m->combo_marker, active);

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker));
    if (gtk_tree_model_iter_n_children(model, NULL))
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui_m->m->combo_marker), 0);
    else
        ui_menu_marker_combo_changed(GTK_COMBO_BOX(ui_m->m->combo_marker), ui_m);
}

void
overlayaz_ui_menu_marker_set_id(overlayaz_ui_menu_marker_t *ui_m,
                                gint                        id)
{
    gtk_combo_box_set_active(GTK_COMBO_BOX(ui_m->m->combo_marker), id - 1);
}

gint
overlayaz_ui_menu_marker_get_id(overlayaz_ui_menu_marker_t *ui_m)
{
    return gtk_combo_box_get_active(GTK_COMBO_BOX(ui_m->m->combo_marker)) + 1;
}


const overlayaz_marker_t*
overlayaz_ui_menu_marker_get_current(overlayaz_ui_menu_marker_t *ui_m)
{
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker));
    GtkTreeIter iter;
    overlayaz_marker_t *m;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return NULL;

    m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
    return m;
}

static void
ui_menu_marker_format_name(GtkCellLayout   *cell_layout,
                           GtkCellRenderer *renderer,
                           GtkTreeModel    *model,
                           GtkTreeIter     *iter,
                           gpointer         user_data)
{
    GtkTreePath *path;
    gchar *name, *tmp;
    gint *n;
    overlayaz_marker_t *m;
    size_t i, len;

    path = gtk_tree_model_get_path(model, iter);
    n = gtk_tree_path_get_indices(path);
    m = overlayaz_marker_list_get(GTK_LIST_STORE(model), iter);

    name = g_strdup_printf("%d. %s", n[0]+1, overlayaz_marker_get_name(m));

    if (g_utf8_strlen(name, -1) > 24+1)
    {
        tmp = g_utf8_substring(name, 0, 24);
        g_free(name);
        name = g_strdup_printf("%s…", tmp);
        g_free(tmp);
    }


    /* Remove new line characters */
    len = strlen(name);
    for (i = 0; i < len; i++)
        if (name[i] == '\n')
            name[i] = ' ';

    g_object_set(renderer,
                 "text", name,
                 "strikethrough", !overlayaz_marker_get_active(m),
                 NULL);

    g_free(name);
    gtk_tree_path_free(path);
}

static void
ui_menu_marker_combo_changed(GtkComboBox                *widget,
                             overlayaz_ui_menu_marker_t *ui_m)
{
    GtkTreeModel *model = gtk_combo_box_get_model(widget);
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(ui_m->m->textview_name));
    overlayaz_marker_t *m;
    GtkTreeIter iter;
    gboolean active;
    gint count;
    static const GdkRGBA color = {0};

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_MAP);

    active = gtk_combo_box_get_active_iter(widget, &iter);
    gtk_widget_set_sensitive(ui_m->m->button_up, active);
    gtk_widget_set_sensitive(ui_m->m->button_down, active);
    gtk_widget_set_sensitive(ui_m->m->button_remove, active);
    gtk_widget_set_sensitive(ui_m->m->textview_name, active);
    gtk_widget_set_sensitive(ui_m->m->spin_lat, active);
    gtk_widget_set_sensitive(ui_m->m->spin_lon, active);
    gtk_widget_set_sensitive(ui_m->m->spin_azi, active);
    gtk_widget_set_sensitive(ui_m->m->spin_dist, active);
    gtk_widget_set_sensitive(ui_m->m->combo_tick, active);
    gtk_widget_set_sensitive(ui_m->m->font_marker, active);
    gtk_widget_set_sensitive(ui_m->m->color_marker_font, active);
    gtk_widget_set_sensitive(ui_m->m->scale_pos, active);
    gtk_widget_set_sensitive(ui_m->m->check_active, active);
    gtk_widget_set_sensitive(ui_m->m->check_show_azi, active);
    gtk_widget_set_sensitive(ui_m->m->check_show_dist, active);
    gtk_widget_set_sensitive(ui_m->m->button_tick_apply, active);
    gtk_widget_set_sensitive(ui_m->m->button_font_apply, active);
    gtk_widget_set_sensitive(ui_m->m->button_pos_apply, active);
    gtk_widget_set_sensitive(ui_m->m->button_active_apply, active);
    gtk_widget_set_sensitive(ui_m->m->button_show_azi_apply, active);
    gtk_widget_set_sensitive(ui_m->m->button_show_dist_apply, active);

    count = gtk_tree_model_iter_n_children(model, NULL);
    gtk_widget_set_sensitive(ui_m->m->button_clear, (count != 0));

    if (!active)
    {
        ui_m->lock = TRUE;
        gtk_text_buffer_set_text(text_buffer, "", -1);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_m->m->spin_lat), 0.0);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_m->m->spin_lon), 0.0);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_m->m->spin_azi), 0.0);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_m->m->spin_dist), 0.0);
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui_m->m->combo_tick), -1);
        gtk_font_chooser_set_font(GTK_FONT_CHOOSER(ui_m->m->font_marker), "");
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(ui_m->m->color_marker_font), &color);
        gtk_range_set_value(GTK_RANGE(ui_m->m->scale_pos), 0.0);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui_m->m->check_active), FALSE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui_m->m->check_show_azi), FALSE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui_m->m->check_show_dist), FALSE);
        ui_m->lock = FALSE;
        return;
    }
    else
    {
        m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);

        ui_m->lock = TRUE;
        gtk_text_buffer_set_text(text_buffer, overlayaz_marker_get_name(m), -1);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_m->m->spin_lat), overlayaz_marker_get_latitude(m));
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_m->m->spin_lon), overlayaz_marker_get_longitude(m));
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui_m->m->combo_tick), overlayaz_marker_get_tick(m));
        gtk_font_chooser_set_font(GTK_FONT_CHOOSER(ui_m->m->font_marker), overlayaz_font_get(overlayaz_marker_get_font(m)));
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(ui_m->m->color_marker_font), overlayaz_marker_get_font_color(m));
        gtk_range_set_value(GTK_RANGE(ui_m->m->scale_pos), overlayaz_marker_get_position(m));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui_m->m->check_active), overlayaz_marker_get_active(m));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui_m->m->check_show_azi), overlayaz_marker_get_show_azimuth(m));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui_m->m->check_show_dist), overlayaz_marker_get_show_distance(m));
        ui_m->lock = FALSE;

        /* Force update of azimuth and distance fields */
        ui_menu_marker_spin_latlon_changed(GTK_SPIN_BUTTON(ui_m->m->spin_lat), ui_m);
    }
}

static void
ui_menu_marker_button_down(GtkButton                  *widget,
                           overlayaz_ui_menu_marker_t *ui_m)
{
    GtkTreeIter iter;
    GtkTreeIter next;
    GtkTreeModel *model;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return;

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker));
    next = iter;

    if (!gtk_tree_model_iter_next(model, &next))
        return;

    gtk_list_store_move_after(GTK_LIST_STORE(model), &iter, &next);
    gtk_widget_queue_draw(ui_m->m->combo_marker);
    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_MAP);
}

static void
ui_menu_marker_button_up(GtkButton                  *widget,
                         overlayaz_ui_menu_marker_t *ui_m)
{
    GtkTreeIter iter;
    GtkTreeIter prev;
    GtkTreePath *path;
    GtkTreeModel *model;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return;

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker));
    path = gtk_tree_model_get_path(model, &iter);
    gtk_tree_path_prev(path);

    if (gtk_tree_model_get_iter(model, &prev, path))
    {
        gtk_list_store_move_before(GTK_LIST_STORE(model), &iter, &prev);
        gtk_widget_queue_draw(ui_m->m->combo_marker);
    }

    gtk_tree_path_free(path);

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_MAP);
}

static void
ui_menu_marker_button_remove(GtkButton                  *widget,
                             overlayaz_ui_menu_marker_t *ui_m)
{
    GtkTreeIter iter;
    GtkTreeIter next;
    GtkTreePath *path;
    GtkTreeModel *model;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return;

    if (!overlayaz_dialog_ask_yesno(overlayaz_ui_get_parent(ui_m->ui),
                                    "Remove marker",
                                    "Do you really want to remove current marker?"))
    {
        return;
    }

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker));
    next = iter;

    if (gtk_tree_model_iter_next(model, &next))
        gtk_combo_box_set_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &next);
    else
    {
        path = gtk_tree_model_get_path(model, &iter);
        gtk_tree_path_prev(path);
        if (gtk_tree_model_get_iter(model, &next, path))
            gtk_combo_box_set_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &next);
        gtk_tree_path_free(path);
    }

    overlayaz_marker_list_remove(GTK_LIST_STORE(model), &iter);
    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE | OVERLAYAZ_UI_UPDATE_MAP);
}

static void
ui_menu_marker_button_clear(GtkButton                  *widget,
                            overlayaz_ui_menu_marker_t *ui_m)
{
    GtkTreeModel *model;

    if (!overlayaz_dialog_ask_yesno(overlayaz_ui_get_parent(ui_m->ui), "Remove all markers", "Do you really want to remove all markers?"))
        return;

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker));
    overlayaz_marker_list_clear(GTK_LIST_STORE(model));
    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE | OVERLAYAZ_UI_UPDATE_MAP);
}

static gboolean
ui_menu_marker_textview_name_key_press(GtkWidget                  *widget,
                                       GdkEventKey                *event,
                                       overlayaz_ui_menu_marker_t *ui_m)
{
    /* Workaround for main window keyboard shortcuts */
    return GDK_EVENT_STOP;
}

static void
ui_menu_marker_textbuffer_name_changed(GtkTextBuffer              *buffer,
                                       overlayaz_ui_menu_marker_t *ui_m)
{
    GtkListStore *model = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker)));
    GtkTreeIter iter;
    overlayaz_marker_t *m;
    GtkTextIter start, end;
    gchar *text;

    if (ui_m->lock)
        return;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return;

    gtk_text_buffer_get_bounds(buffer, &start, &end);
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
    overlayaz_marker_set_name(m, text);
    overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);

    g_free(text);
    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
}

static void
ui_menu_marker_spin_lat_changed(GtkSpinButton              *widget,
                                overlayaz_ui_menu_marker_t *ui_m)
{

    GtkListStore *model = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker)));
    GtkTreeIter iter;
    overlayaz_marker_t *m;

    if (ui_m->lock)
        return;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return;

    m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
    overlayaz_marker_set_latitude(m, gtk_spin_button_get_value(widget));
    overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE | OVERLAYAZ_UI_UPDATE_MAP);
}

static void
ui_menu_marker_spin_lon_changed(GtkSpinButton              *widget,
                                overlayaz_ui_menu_marker_t *ui_m)
{
    GtkListStore *model = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker)));
    GtkTreeIter iter;
    overlayaz_marker_t *m;

    if (ui_m->lock)
        return;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return;

    m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
    overlayaz_marker_set_longitude(m, gtk_spin_button_get_value(widget));
    overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE | OVERLAYAZ_UI_UPDATE_MAP);
}

static void
ui_menu_marker_combo_changed_tick(GtkComboBox                *widget,
                                  overlayaz_ui_menu_marker_t *ui_m)
{
    GtkListStore *model = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker)));
    GtkTreeIter iter;
    overlayaz_marker_t *m;

    if (ui_m->lock)
        return;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return;

    m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
    overlayaz_marker_set_tick(m, gtk_combo_box_get_active(widget));
    overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
}

static void
ui_menu_marker_font_set(GtkFontButton              *widget,
                        overlayaz_ui_menu_marker_t *ui_m)
{
    GtkListStore *model = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker)));
    GtkTreeIter iter;
    overlayaz_marker_t *m;
    gchar *font;

    if (ui_m->lock)
        return;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return;

    font = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(widget));

    m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
    overlayaz_marker_set_font(m, font);
    overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);

    g_free(font);
    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
}

static void
ui_menu_marker_font_color_set(GtkColorButton             *widget,
                              overlayaz_ui_menu_marker_t *ui_m)
{
    GtkListStore *model = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker)));
    GtkTreeIter iter;
    GdkRGBA color;
    overlayaz_marker_t *m;

    if (ui_m->lock)
        return;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return;

    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &color);
    m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
    overlayaz_marker_set_font_color_rgba(m, &color);
    overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
}

static void
ui_menu_marker_scale_pos_changed(GtkRange                   *widget,
                                 overlayaz_ui_menu_marker_t *ui_m)
{
    GtkListStore *model = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker)));
    GtkTreeIter iter;
    overlayaz_marker_t *m;

    if (ui_m->lock)
        return;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return;

    m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
    overlayaz_marker_set_position(m, gtk_range_get_value(widget));
    overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
}

static void
ui_menu_marker_check_active_toggled(GtkToggleButton            *widget,
                                    overlayaz_ui_menu_marker_t *ui_m)
{
    GtkListStore *model = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker)));
    GtkTreeIter iter;
    overlayaz_marker_t *m;

    if (ui_m->lock)
        return;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return;

    m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
    overlayaz_marker_set_active(m, gtk_toggle_button_get_active(widget));
    overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE | OVERLAYAZ_UI_UPDATE_MAP);
}

static void
ui_menu_marker_check_show_azi_toggled(GtkToggleButton            *widget,
                                      overlayaz_ui_menu_marker_t *ui_m)
{
    GtkListStore *model = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker)));
    GtkTreeIter iter;
    overlayaz_marker_t *m;

    if (ui_m->lock)
        return;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return;

    m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
    overlayaz_marker_set_show_azimuth(m, gtk_toggle_button_get_active(widget));
    overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
}

static void
ui_menu_marker_check_show_dist_toggled(GtkToggleButton            *widget,
                                       overlayaz_ui_menu_marker_t *ui_m)
{
    GtkListStore *model = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker)));
    GtkTreeIter iter;
    overlayaz_marker_t *m;

    if (ui_m->lock)
        return;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return;

    m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
    overlayaz_marker_set_show_distance(m, gtk_toggle_button_get_active(widget));
    overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
}

static void
ui_menu_marker_spin_latlon_changed(GtkSpinButton              *widget,
                                   overlayaz_ui_menu_marker_t *ui_m)
{
    struct overlayaz_location home;
    gdouble lat, lon;
    gdouble azi, dist;
    GtkTreeIter iter;

    if (ui_m->lock)
        return;

    if (!gtk_combo_box_get_active_iter(GTK_COMBO_BOX(ui_m->m->combo_marker), &iter))
        return;

    if (!overlayaz_get_location(ui_m->o, &home))
        return;

    lat = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_m->m->spin_lat));
    lon = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_m->m->spin_lon));
    overlayaz_geo_inverse(home.latitude, home.longitude, lat, lon, &azi, NULL, &dist);

    ui_m->lock = TRUE;
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_m->m->spin_azi), azi);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_m->m->spin_dist), dist/1000.0);
    ui_m->lock = FALSE;
}

static void
ui_menu_marker_spin_azidist_changed(GtkSpinButton              *widget,
                                    overlayaz_ui_menu_marker_t *ui_m)
{
    struct overlayaz_location home;
    gdouble new_lat, new_lon;
    gdouble azi, dist;

    if (ui_m->lock)
        return;

    if (!overlayaz_get_location(ui_m->o, &home))
        return;

    azi = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_m->m->spin_azi));
    dist = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_m->m->spin_dist));
    overlayaz_geo_direct(home.latitude, home.longitude, azi,
                         dist*1000.0,
                         &new_lat, &new_lon);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_m->m->spin_lat), new_lat);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_m->m->spin_lon), new_lon);
}

static void
ui_menu_marker_button_tick_apply(GtkButton                  *widget,
                                 overlayaz_ui_menu_marker_t *ui_m)
{
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker));
    gint value = gtk_combo_box_get_active(GTK_COMBO_BOX(ui_m->m->combo_tick));
    GtkTreeIter iter;
    overlayaz_marker_t *m;

    if (!gtk_tree_model_get_iter_first(model, &iter))
        return;

    if (!ui_marker_dialog_apply(ui_m->ui, "tick"))
        return;

    do
    {
        m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
        overlayaz_marker_set_tick(m, value);
        overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);
    } while (gtk_tree_model_iter_next(model, &iter));

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
}

static void
ui_menu_marker_button_font_apply(GtkButton                  *widget,
                                 overlayaz_ui_menu_marker_t *ui_m)
{
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker));
    gchar *value;
    GdkRGBA color;
    GtkTreeIter iter;
    overlayaz_marker_t *m;

    if (!gtk_tree_model_get_iter_first(model, &iter))
        return;

    if (!ui_marker_dialog_apply(ui_m->ui, "font"))
        return;

    value = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(ui_m->m->font_marker));
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(ui_m->m->color_marker_font), &color);

    do
    {
        m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
        overlayaz_marker_set_font(m, value);
        overlayaz_marker_set_font_color_rgba(m, &color);
        overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);
    } while (gtk_tree_model_iter_next(model, &iter));

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
    g_free(value);
}

static void
ui_menu_marker_button_pos_apply(GtkButton                  *widget,
                                overlayaz_ui_menu_marker_t *ui_m)
{

    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker));
    gdouble value = gtk_range_get_value(GTK_RANGE(ui_m->m->scale_pos));
    GtkTreeIter iter;
    overlayaz_marker_t *m;

    if (!gtk_tree_model_get_iter_first(model, &iter))
        return;

    if (!ui_marker_dialog_apply(ui_m->ui, "position"))
        return;

    do
    {
        m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
        overlayaz_marker_set_position(m, value);
        overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);
    } while (gtk_tree_model_iter_next(model, &iter));

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
}

static void
ui_menu_marker_button_active_apply(GtkButton                  *widget,
                                   overlayaz_ui_menu_marker_t *ui_m)
{
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker));
    gboolean value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ui_m->m->check_active));
    GtkTreeIter iter;
    overlayaz_marker_t *m;

    if (!gtk_tree_model_get_iter_first(model, &iter))
        return;

    if (!ui_marker_dialog_apply(ui_m->ui, "active flag"))
        return;

    do
    {
        m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
        overlayaz_marker_set_active(m, value);
        overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);
    } while (gtk_tree_model_iter_next(model, &iter));

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE | OVERLAYAZ_UI_UPDATE_MAP);
}

static void
ui_menu_marker_button_show_azi_apply(GtkButton                  *widget,
                                     overlayaz_ui_menu_marker_t *ui_m)
{
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker));
    gboolean value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ui_m->m->check_show_azi));
    GtkTreeIter iter;
    overlayaz_marker_t *m;

    if (!gtk_tree_model_get_iter_first(model, &iter))
        return;

    if (!ui_marker_dialog_apply(ui_m->ui, "show azimuth flag"))
        return;

    do
    {
        m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
        overlayaz_marker_set_show_azimuth(m, value);
        overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);
    } while (gtk_tree_model_iter_next(model, &iter));

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
}

static void
ui_menu_marker_button_show_dist_apply(GtkButton                  *widget,
                                      overlayaz_ui_menu_marker_t *ui_m)
{
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(ui_m->m->combo_marker));
    gboolean value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ui_m->m->check_show_dist));
    GtkTreeIter iter;
    overlayaz_marker_t *m;

    if (!gtk_tree_model_get_iter_first(model, &iter))
        return;

    if (!ui_marker_dialog_apply(ui_m->ui, "show distance flag"))
        return;

    do
    {
        m = overlayaz_marker_list_get(GTK_LIST_STORE(model), &iter);
        overlayaz_marker_set_show_distance(m, value);
        overlayaz_marker_list_update(GTK_LIST_STORE(model), &iter);
    } while (gtk_tree_model_iter_next(model, &iter));

    overlayaz_ui_update_view(ui_m->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
}

static gboolean
ui_marker_dialog_apply(overlayaz_ui_t *ui,
                       const gchar    *value)
{
    gchar *text = g_strdup_printf("Do you want to set the same\n<b>%s</b> for all markers?", value);
    gboolean ret = overlayaz_dialog_ask_yesno(overlayaz_ui_get_parent(ui),
                                              "Apply value for all markers",
                                              text);
    g_free(text);
    return ret;
}
