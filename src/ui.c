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
#include <math.h>
#include "overlayaz.h"
#include "ui.h"
#include "ui-menu-grid.h"
#include "ui-menu-ref.h"
#include "ui-menu-marker.h"
#include "ui-view-img.h"
#include "ui-view-map.h"
#include "dialog.h"
#include "dialog-export.h"
#include "ui-preferences.h"
#include "conf.h"
#include "export.h"
#include "file.h"
#include "profile.h"
#include "marker-list.h"
#include "ui-util.h"
#include "geo.h"
#include "dialog-info.h"
#include "dialog-about.h"

struct overlayaz_ui
{
    struct overlayaz_window w;
    overlayaz_t *o;
    overlayaz_ui_menu_ref_t *r;
    overlayaz_ui_menu_grid_t *g;
    overlayaz_ui_menu_marker_t *m;
    overlayaz_ui_view_img_t *img;
    overlayaz_ui_view_map_t *map;
    gboolean lock;
    gboolean queue_map_update;
};

static void ui_sync(overlayaz_ui_t*, gboolean);

static void ui_file_chooser_set(GtkFileChooserButton*, overlayaz_ui_t*);
static void ui_menu_page_switched(GtkNotebook*, GtkWidget*, guint, overlayaz_ui_t*);
static void ui_button_save_clicked(GtkButton*, overlayaz_ui_t*);
static void ui_button_export_clicked(GtkButton*, overlayaz_ui_t*);
static void ui_button_preferences_clicked(GtkButton*, overlayaz_ui_t*);
static void ui_button_about_clicked(GtkButton*, overlayaz_ui_t*);
static void ui_view_page_switched(GtkNotebook*, GtkWidget*, guint, overlayaz_ui_t*);
static void ui_rotation_changed(GtkRange*, overlayaz_ui_t*);
static gboolean ui_rotation_scroll_event(GtkWidget*, GdkEventScroll*, overlayaz_ui_t*);
static void ui_button_rotation_reset_clicked(GtkButton*, overlayaz_ui_t*);
static gboolean ui_key_press(GtkWidget*, GdkEventKey*, overlayaz_ui_t*);
static gboolean ui_delete_event(GtkWidget*, GdkEvent*, overlayaz_ui_t*);
static void ui_destroy(GtkWidget*, overlayaz_ui_t*);
static void ui_drag_data_received(GtkWidget*, GdkDragContext*, gint, gint, GtkSelectionData*, guint, guint, overlayaz_ui_t*);

static void ui_add_marker(overlayaz_ui_t*, gdouble, gdouble, gdouble);
static void ui_set_label_value(GtkLabel*, const gchar*);

void
overlayaz_ui(overlayaz_t *o)
{
    overlayaz_ui_t *ui = g_malloc0(sizeof(overlayaz_ui_t));
    static const GtkTargetEntry drop_types[] = {{ "text/uri-list", 0, 0 }};
    static const gint n_drop_types = sizeof(drop_types) / sizeof(drop_types[0]);
    gchar *path;

    /* Initialization */
    overlayaz_window_init(&ui->w);
    ui->o = o;
    ui->r = overlayaz_ui_menu_ref_new(ui, &ui->w.r, o);
    ui->g = overlayaz_ui_menu_grid_new(ui, &ui->w.g, o);
    ui->m = overlayaz_ui_menu_marker_new(ui, &ui->w.m, o);
    ui->img = overlayaz_ui_view_img_new(ui, ui->w.image, o);
    ui->map = overlayaz_ui_view_map_new(ui, ui->w.map, o);

    /* Events */
    g_signal_connect(ui->w.file_chooser, "file-set", G_CALLBACK(ui_file_chooser_set), ui);
    g_signal_connect(ui->w.notebook_menu, "switch-page", G_CALLBACK(ui_menu_page_switched), ui);
    g_signal_connect(ui->w.button_save, "clicked", G_CALLBACK(ui_button_save_clicked), ui);
    g_signal_connect(ui->w.button_export, "clicked", G_CALLBACK(ui_button_export_clicked), ui);
    g_signal_connect(ui->w.button_preferences, "clicked", G_CALLBACK(ui_button_preferences_clicked), ui);
    g_signal_connect(ui->w.button_about, "clicked", G_CALLBACK(ui_button_about_clicked), ui);
    g_signal_connect(ui->w.notebook_view, "switch-page", G_CALLBACK(ui_view_page_switched), ui);
    g_signal_connect(ui->w.scale_rotation, "value-changed", G_CALLBACK(ui_rotation_changed), ui);
    g_signal_connect(ui->w.scale_rotation, "scroll-event", G_CALLBACK(ui_rotation_scroll_event), NULL);
    g_signal_connect(ui->w.button_rotation_reset, "clicked", G_CALLBACK(ui_button_rotation_reset_clicked), ui);

    g_signal_connect_after(ui->w.window, "key-press-event", G_CALLBACK(ui_key_press), ui);
    g_signal_connect(ui->w.window, "delete-event", G_CALLBACK(ui_delete_event), ui);
    g_signal_connect(ui->w.window, "destroy", G_CALLBACK(ui_destroy), ui);

    /* Drag and drop support */
    gtk_drag_dest_set(ui->w.window, GTK_DEST_DEFAULT_ALL, drop_types, n_drop_types, GDK_ACTION_COPY);
    g_signal_connect(ui->w.window, "drag-data-received", G_CALLBACK(ui_drag_data_received), ui);

    path = overlayaz_conf_get_open_path();
    if (path && strlen(path))
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(ui->w.file_chooser), path);
    g_free(path);

    /* Synchronize UI with the model */
    ui_sync(ui, FALSE);
}

GtkWindow*
overlayaz_ui_get_parent(overlayaz_ui_t *ui)
{
    return GTK_WINDOW(ui->w.window);
}

void
overlayaz_ui_update_view(overlayaz_ui_t                *ui,
                         enum overlayaz_ui_update_mask  mode)
{
    gint current_view = overlayaz_ui_get_view(ui);

    if (mode & OVERLAYAZ_UI_UPDATE_IMAGE)
        if (current_view == OVERLAYAZ_WINDOW_VIEW_IMAGE)
            gtk_widget_queue_draw(ui->w.image);

    if (mode & OVERLAYAZ_UI_UPDATE_MAP)
    {
        if (current_view == OVERLAYAZ_WINDOW_VIEW_MAP)
            overlayaz_ui_view_map_update(ui->map);
        else
            ui->queue_map_update = TRUE;
    }
}

void
overlayaz_ui_set_menu(overlayaz_ui_t              *ui,
                      enum overlayaz_window_menu   id)
{
    gtk_notebook_set_current_page(GTK_NOTEBOOK(ui->w.notebook_menu), id);
}

enum overlayaz_window_menu
overlayaz_ui_get_menu(overlayaz_ui_t *ui)
{
    return gtk_notebook_get_current_page(GTK_NOTEBOOK(ui->w.notebook_menu));
}

void
overlayaz_ui_set_view(overlayaz_ui_t              *ui,
                      enum overlayaz_window_view   id)
{
    gtk_notebook_set_current_page(GTK_NOTEBOOK(ui->w.notebook_view), id);
}

enum overlayaz_window_view
overlayaz_ui_get_view(overlayaz_ui_t *ui)
{
    return gtk_notebook_get_current_page(GTK_NOTEBOOK(ui->w.notebook_view));
}

void
overlayaz_ui_show_azimuth(overlayaz_ui_t *ui,
                          gdouble         value)
{
    gchar *text = overlayaz_ui_util_format_angle(value);
    ui_set_label_value(GTK_LABEL(ui->w.label_meas_first_value), text);
    g_free(text);
}

void
overlayaz_ui_show_elevation(overlayaz_ui_t *ui,
                            gdouble         value)
{
    gchar *text = overlayaz_ui_util_format_angle(value);
    ui_set_label_value(GTK_LABEL(ui->w.label_meas_second_value), text);
    g_free(text);
}

void
overlayaz_ui_show_distance(overlayaz_ui_t *ui,
                           gdouble         value)
{
    gchar *text = overlayaz_ui_util_format_distance(value);
    ui_set_label_value(GTK_LABEL(ui->w.label_meas_second_value), text);
    g_free(text);
}

gboolean
overlayaz_ui_get_ref(const overlayaz_ui_t    *ui,
                     enum overlayaz_ref_type *ref_type,
                     enum overlayaz_ref_id   *ref_id)
{
    return overlayaz_ui_menu_ref_get(ui->r, ref_type, ref_id);
}

gint
overlayaz_ui_get_marker_id(overlayaz_ui_t *ui)
{
    return overlayaz_ui_menu_marker_get_id(ui->m);
}


void
overlayaz_ui_set_rotation(overlayaz_ui_t *ui,
                          gdouble         value)
{
    gtk_range_set_value(GTK_RANGE(ui->w.scale_rotation), value);
}

void
overlayaz_ui_action_position(overlayaz_ui_t           *ui,
                             enum overlayaz_ui_action  action,
                             gdouble                   pos_x,
                             gdouble                   pos_y)
{
    struct overlayaz_location home;
    gdouble latitude = NAN;
    gdouble longitude = NAN;
    gdouble azimuth = NAN;
    gdouble elevation = NAN;
    gdouble distance;

    if (action == OVERLAYAZ_UI_ACTION_SET)
    {
        if (overlayaz_ui_get_menu(ui) == OVERLAYAZ_WINDOW_MENU_REF)
        {
            if (overlayaz_ui_menu_ref_get(ui->r, NULL, NULL))
                overlayaz_ui_menu_ref_set_position(ui->r, pos_x, pos_y);
            else
                overlayaz_dialog(overlayaz_ui_get_parent(ui),
                                 GTK_MESSAGE_WARNING,
                                 "Reference",
                                 "No reference selected.");
        }
        else if (overlayaz_ui_get_menu(ui) == OVERLAYAZ_WINDOW_MENU_MARKER)
        {
            if (overlayaz_get_location(ui->o, &home) &&
                overlayaz_get_angle(ui->o, OVERLAYAZ_REF_AZ, pos_x, &azimuth))
            {
                distance = overlayaz_conf_get_map_grid_distance();
                overlayaz_geo_direct(home.latitude, home.longitude, azimuth,
                                     distance * 1000.0,
                                     &latitude, &longitude);
                ui_add_marker(ui, latitude, longitude, pos_y);
            }
        }
    }
    else if (action == OVERLAYAZ_UI_ACTION_INFO)
    {
        if (overlayaz_get_location(ui->o, &home))
        {
            overlayaz_get_angle(ui->o, OVERLAYAZ_REF_AZ, pos_x, &azimuth);
            overlayaz_get_angle(ui->o, OVERLAYAZ_REF_EL, pos_y, &elevation);
        }

        overlayaz_dialog_info(overlayaz_ui_get_parent(ui), pos_x, pos_y, latitude, longitude, azimuth, elevation, NAN);
    }
}

void
overlayaz_ui_action_location(overlayaz_ui_t           *ui,
                             enum overlayaz_ui_action  action,
                             gdouble                   latitude,
                             gdouble                   longitude)
{
    struct overlayaz_location home;
    gdouble azimuth = NAN;
    gdouble distance = NAN;
    gdouble pos_x = NAN;

    if (action == OVERLAYAZ_UI_ACTION_SET)
    {
        if (overlayaz_ui_get_menu(ui) == OVERLAYAZ_WINDOW_MENU_REF)
        {
            if (overlayaz_ui_menu_ref_get(ui->r, NULL, NULL))
                overlayaz_ui_menu_ref_set_location(ui->r, latitude, longitude);
            else
                overlayaz_dialog(overlayaz_ui_get_parent(ui),
                                 GTK_MESSAGE_WARNING,
                                 "Reference",
                                 "No reference selected.");
        }
        else if (overlayaz_ui_get_menu(ui) == OVERLAYAZ_WINDOW_MENU_MARKER)
        {
            if (overlayaz_get_location(ui->o, &home))
            {
                overlayaz_geo_inverse(home.latitude, home.longitude,
                                      latitude, longitude,
                                      &azimuth, NULL, NULL);

                ui_add_marker(ui, latitude, longitude, NAN);
            }
        }
    }
    else if (action == OVERLAYAZ_UI_ACTION_INFO)
    {
        if (overlayaz_get_location(ui->o, &home))
        {
            overlayaz_geo_inverse(home.latitude, home.longitude,
                                  latitude, longitude,
                                  &azimuth, NULL, &distance);
            overlayaz_get_position(ui->o, OVERLAYAZ_REF_AZ, azimuth, &pos_x);
        }

        overlayaz_dialog_info(overlayaz_ui_get_parent(ui), pos_x, NAN, latitude, longitude, azimuth, NAN, distance);
    }
}

void
overlayaz_ui_set_map_source(overlayaz_ui_t *ui,
                            gint            source_id)
{
    overlayaz_ui_view_map_set_source(ui->map, source_id);
}

static void
ui_sync(overlayaz_ui_t *ui,
        gboolean        file_only)
{
    const gchar *filename;
    gboolean active;

    /* Lock model update callbacks */
    ui->lock = TRUE;

    filename = overlayaz_get_filename(ui->o);
    if (filename == NULL)
    {
        gtk_file_chooser_unselect_all(GTK_FILE_CHOOSER(ui->w.file_chooser));
        active = FALSE;
    }
    else
    {
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(ui->w.file_chooser), filename);
        active = TRUE;
    }

    if (file_only)
    {
        ui->lock = FALSE;
        return;
    }

    gtk_range_set_value(GTK_RANGE(ui->w.scale_rotation), overlayaz_get_rotation(ui->o));
    ui->lock = FALSE;

    overlayaz_ui_set_view(ui, OVERLAYAZ_WINDOW_VIEW_IMAGE);

    gtk_widget_set_sensitive(ui->w.button_save, active);
    gtk_widget_set_sensitive(ui->w.button_export, active);
    gtk_widget_set_sensitive(ui->w.scale_rotation, active);
    gtk_widget_set_sensitive(ui->w.button_rotation_reset, active);

    overlayaz_ui_menu_ref_sync(ui->r, active);
    overlayaz_ui_menu_grid_sync(ui->g, active);
    overlayaz_ui_menu_marker_sync(ui->m, active);
    overlayaz_ui_view_map_sync(ui->map, active);
    overlayaz_ui_view_img_sync(ui->img, active);

    overlayaz_ui_show_azimuth(ui, NAN);
    overlayaz_ui_show_elevation(ui, NAN);

    /* Force redraw */
    overlayaz_ui_update_view(ui, OVERLAYAZ_UI_UPDATE_IMAGE | OVERLAYAZ_UI_UPDATE_MAP);
}

static void
ui_file_chooser_set(GtkFileChooserButton *widget,
                    overlayaz_ui_t       *ui)
{
    enum overlayaz_file_load_error error;
    gchar *filename;
    gchar *path;

    if (ui->lock)
        return;

    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
    if (filename == NULL)
    {
        overlayaz_reset(ui->o);
        ui_sync(ui, FALSE);
        return;
    }

    if (overlayaz_changed(ui->o))
    {
        /* Revert previous filename, as we should ask what to do with it */
        ui_sync(ui, TRUE);

        switch (overlayaz_dialog_ask_unsaved(overlayaz_ui_get_parent(ui)))
        {
        case GTK_RESPONSE_YES:
            gtk_button_clicked(GTK_BUTTON(ui->w.button_save));
            if (overlayaz_changed(ui->o))
            {
                g_free(filename);
                return;
            }
            break;
        case GTK_RESPONSE_NO:
            break;
        default:
            g_free(filename);
            return;
        }
    }

    error = overlayaz_file_load(ui->o, filename);

    /* Sync UI with the model */
    ui_sync(ui, FALSE);

    switch (error)
    {
    case OVERLAYAZ_FILE_LOAD_ERROR_IMAGE_OPEN:
        overlayaz_dialog(overlayaz_ui_get_parent(ui), GTK_MESSAGE_ERROR, "Profile", "Failed to open the image");
        break;
    case OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_OPEN:
        overlayaz_dialog(overlayaz_ui_get_parent(ui), GTK_MESSAGE_ERROR, "Profile", "Failed to open the profile file");
        break;
    case OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_PARSE:
        overlayaz_dialog(overlayaz_ui_get_parent(ui), GTK_MESSAGE_ERROR, "Profile", "Failed to parse the profile file");
        break;
    case OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_FORMAT:
        overlayaz_dialog(overlayaz_ui_get_parent(ui), GTK_MESSAGE_ERROR, "Profile", "The profile file has an invalid format");
        break;
    case OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_VERSION:
        overlayaz_dialog(overlayaz_ui_get_parent(ui), GTK_MESSAGE_ERROR, "Profile", "The profile format version is unsupported");
        break;
    case OVERLAYAZ_FILE_LOAD_OK:
    default:
        break;
    }

    path = g_path_get_dirname(filename);
    overlayaz_conf_set_open_path(path);
    g_free(path);
    g_free(filename);
}

static void
ui_menu_page_switched(GtkNotebook    *notebook,
                      GtkWidget      *page,
                      guint           page_num,
                      overlayaz_ui_t *ui)
{
    if (page_num != OVERLAYAZ_WINDOW_MENU_REF)
    {
        overlayaz_ui_menu_ref_none(ui->r);
        overlayaz_ui_update_view(ui, OVERLAYAZ_UI_UPDATE_IMAGE);
    }
}

static void
ui_button_save_clicked(GtkButton      *button,
                       overlayaz_ui_t *ui)
{
    gchar *filename_profile;
    enum overlayaz_profile_save_error error;
    gint response;

    if (!overlayaz_get_filename(ui->o))
        return;

    filename_profile = g_strdup_printf("%s%s", overlayaz_get_filename(ui->o), OVERLAYAZ_EXTENSION_PROFILE);
    if (g_file_test(filename_profile, G_FILE_TEST_EXISTS))
    {
        response = overlayaz_dialog_ask_overwrite(overlayaz_ui_get_parent(ui));
        if (response != GTK_RESPONSE_YES)
        {
            g_free(filename_profile);
            return;
        }
    }

    error = overlayaz_profile_save(ui->o, filename_profile);
    switch(error)
    {
    case OVERLAYAZ_PROFILE_SAVE_OK:
        overlayaz_unchanged(ui->o);
        break;

    case OVERLAYAZ_PROFILE_SAVE_ERROR_OPEN:
    case OVERLAYAZ_PROFILE_SAVE_ERROR_WRITE:
    default:
        overlayaz_dialog(overlayaz_ui_get_parent(ui), GTK_MESSAGE_ERROR, "Profile", "Failed to save the profile file");
        break;
    }

    g_free(filename_profile);
}

static void
ui_button_export_clicked(GtkButton      *button,
                         overlayaz_ui_t *ui)
{
    overlayaz_dialog_export_t *e = overlayaz_dialog_export(overlayaz_ui_get_parent(ui));

    if (overlayaz_dialog_export_get_filename(e))
    {
        overlayaz_export(ui->o,
                         overlayaz_dialog_export_get_filename(e),
                         overlayaz_dialog_export_get_filter(e),
                         overlayaz_dialog_export_get_quality(e));
    }

    overlayaz_dialog_export_free(e);
}

static void
ui_button_preferences_clicked(GtkButton      *button,
                              overlayaz_ui_t *ui)
{
    overlayaz_ui_preferences(ui, ui->o);
}

static void
ui_button_about_clicked(GtkButton      *button,
                        overlayaz_ui_t *ui)
{
    overlayaz_dialog_about(overlayaz_ui_get_parent(ui));
}

static void
ui_view_page_switched(GtkNotebook    *notebook,
                      GtkWidget      *page,
                      guint           page_num,
                      overlayaz_ui_t *ui)
{
    if (page_num == OVERLAYAZ_WINDOW_VIEW_IMAGE)
    {
        gtk_label_set_text(GTK_LABEL(ui->w.label_meas_second), "Elevation:");
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(ui->w.label_meas_second), "Distance:");
        if (ui->queue_map_update)
        {
            overlayaz_ui_view_map_update(ui->map);
            ui->queue_map_update = FALSE;
        }
    }
}

static void
ui_rotation_changed(GtkRange       *range,
                    overlayaz_ui_t *ui)
{
    if (!ui->lock)
    {
        overlayaz_set_rotation(ui->o, gtk_range_get_value(range));
        overlayaz_ui_update_view(ui, OVERLAYAZ_UI_UPDATE_IMAGE);
    }
}

static gboolean
ui_rotation_scroll_event(GtkWidget      *widget,
                         GdkEventScroll *event,
                         overlayaz_ui_t *ui)
{
    if (!gtk_widget_has_focus(widget))
        return GDK_EVENT_STOP;

    return GDK_EVENT_PROPAGATE;
}

static void
ui_button_rotation_reset_clicked(GtkButton      *button,
                                 overlayaz_ui_t *ui)
{
    gtk_range_set_value(GTK_RANGE(ui->w.scale_rotation), 0.0);
}

static gboolean
ui_key_press(GtkWidget      *widget,
             GdkEventKey    *event,
             overlayaz_ui_t *ui)
{
    switch (gdk_keyval_to_upper(event->keyval))
    {
    case GDK_KEY_Escape:
        if (overlayaz_ui_get_menu(ui) == OVERLAYAZ_WINDOW_MENU_REF)
        {
            overlayaz_ui_menu_ref_none(ui->r);
            overlayaz_ui_update_view(ui, OVERLAYAZ_UI_UPDATE_IMAGE);
        }
        break;

    case GDK_KEY_1:
        if (event->state & GDK_MOD1_MASK)
            overlayaz_ui_set_view(ui, OVERLAYAZ_WINDOW_VIEW_IMAGE);
        break;

    case GDK_KEY_2:
        if (event->state & GDK_MOD1_MASK)
            overlayaz_ui_set_view(ui, OVERLAYAZ_WINDOW_VIEW_MAP);
        break;

    case GDK_KEY_Z:
        /* Set zoom level to 100% */
        if (overlayaz_ui_get_view(ui) == OVERLAYAZ_WINDOW_VIEW_IMAGE)
            overlayaz_ui_view_img_zoom_in(ui->img);
        break;

    case GDK_KEY_X:
        /* Set zoom level to fit the window */
        if (overlayaz_ui_get_view(ui) == OVERLAYAZ_WINDOW_VIEW_IMAGE)
            overlayaz_ui_view_img_zoom_fit(ui->img);
        break;
    }

    return GDK_EVENT_PROPAGATE;
}

static gboolean
ui_delete_event(GtkWidget      *widget,
                GdkEvent       *event,
                overlayaz_ui_t *ui)
{
    if (!overlayaz_changed(ui->o))
        return GDK_EVENT_PROPAGATE;

    switch (overlayaz_dialog_ask_unsaved(overlayaz_ui_get_parent(ui)))
    {
    case GTK_RESPONSE_YES:
        gtk_button_clicked(GTK_BUTTON(ui->w.button_save));
        return overlayaz_changed(ui->o);
    case GTK_RESPONSE_NO:
        return GDK_EVENT_PROPAGATE;
    default:
        return GDK_EVENT_STOP;
    }
}

static void
ui_destroy(GtkWidget      *widget,
           overlayaz_ui_t *ui)
{
    overlayaz_ui_menu_ref_free(ui->r);
    overlayaz_ui_menu_grid_free(ui->g);
    overlayaz_ui_menu_marker_free(ui->m);
    overlayaz_ui_view_img_free(ui->img);
    overlayaz_ui_view_map_free(ui->map);
    g_free(ui);
    gtk_main_quit();
}

static void
ui_drag_data_received(GtkWidget        *widget,
                      GdkDragContext   *context,
                      gint              x,
                      gint              y,
                      GtkSelectionData *selection_data,
                      guint             info,
                      guint             time,
                      overlayaz_ui_t   *ui)
{
    gchar **list;
    gchar *filename;

    if (selection_data == NULL)
        return;

    if (info != 0)
        return;

    list = gtk_selection_data_get_uris(selection_data);
    if (list && *list)
    {
        filename = g_filename_from_uri(*list, NULL, NULL);
        if (filename)
        {
            gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(ui->w.file_chooser), filename);
            ui_file_chooser_set(GTK_FILE_CHOOSER_BUTTON(ui->w.file_chooser), ui);
        }

        g_free(filename);
        g_strfreev(list);
    }
}

static void
ui_add_marker(overlayaz_ui_t *ui,
              gdouble         lat,
              gdouble         lon,
              gdouble         pos_y)
{

    GtkListStore *ml = overlayaz_get_marker_list(ui->o);
    gint count = overlayaz_marker_list_count(ml);
    const overlayaz_marker_t *m_curr = overlayaz_ui_menu_marker_get_current(ui->m);
    overlayaz_marker_t *m;

    m = overlayaz_marker_new();
    overlayaz_marker_set_latitude(m, lat);
    overlayaz_marker_set_longitude(m, lon);

    if (!isnan(pos_y))
        overlayaz_marker_set_position(m, pos_y / (gdouble)overlayaz_get_height(ui->o) * 100.0);
    else if (m_curr)
        overlayaz_marker_set_position(m, overlayaz_marker_get_position(m_curr));

    if (m_curr)
    {
        overlayaz_marker_set_tick(m, overlayaz_marker_get_tick(m_curr));
        overlayaz_marker_set_font(m, overlayaz_font_get(overlayaz_marker_get_font(m_curr)));
        overlayaz_marker_set_font_color_rgba(m, overlayaz_marker_get_font_color(m_curr));
        overlayaz_marker_set_show_azimuth(m, overlayaz_marker_get_show_azimuth(m_curr));
        overlayaz_marker_set_show_distance(m, overlayaz_marker_get_show_distance(m_curr));
    }

    overlayaz_marker_list_add(overlayaz_get_marker_list(ui->o), m);

    overlayaz_ui_set_menu(ui, OVERLAYAZ_WINDOW_MENU_MARKER);
    overlayaz_ui_menu_marker_set_id(ui->m, count + 1);
    overlayaz_ui_update_view(ui, OVERLAYAZ_UI_UPDATE_IMAGE | OVERLAYAZ_UI_UPDATE_MAP);
}

static void
ui_set_label_value(GtkLabel    *label,
                   const gchar *text)
{
    gchar *markup = g_strdup_printf("<span font_desc=\"monospace\" font_size=\"xx-large\">%s</span>", text);
    gtk_label_set_markup(label, markup);
    g_free(markup);
}
