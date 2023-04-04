/*
 *  overlayaz – photo visibility analysis software
 *  Copyright (c) 2020-2023  Konrad Kosmatka
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
#include <osm-gps-map-source.h>
#include "ui-preferences.h"
#include "conf.h"
#ifdef G_OS_WIN32
#include "mingw.h"
#endif

typedef struct overlayaz_dialog_prefs
{
    const overlayaz_t *o;
    GtkWidget *dialog;
    GtkWidget *grid;
    GtkWidget *label_srtm;
    GtkWidget *file_chooser_srtm;
    GtkWidget *label_map_source;
    GtkWidget *combo_map_source;
    GtkWidget *label_map_grid_distance;
    GtkWidget *spin_map_grid_distance;
    GtkWidget *label_location;
    GtkWidget *button_location;
    GtkWidget *label_latitude;
    GtkWidget *spin_latitude;
    GtkWidget *label_longitude;
    GtkWidget *spin_longitude;
    GtkWidget *label_altitude;
    GtkWidget *spin_altitude;
    GtkWidget *check_dark_theme;
    gboolean map_update;
    gboolean map_source_change;
} overlayaz_dialog_prefs_t;

static GtkTreeModel* ui_preferences_map_sources(void);
static void ui_preferences_map_source_render(GtkCellLayout*, GtkCellRenderer*, GtkTreeModel*, GtkTreeIter*, gpointer);
static void ui_preferences_from_config(overlayaz_dialog_prefs_t*);
static void ui_preferences_to_config(overlayaz_dialog_prefs_t*);
static void ui_preferences_combo_map_source_changed(GtkComboBox*, gpointer);
static void ui_preferences_spin_map_grid_distance_changed(GtkSpinButton*, gpointer);
static void ui_preferences_button_location_clicked(GtkButton*, gpointer);


void
overlayaz_ui_preferences(overlayaz_ui_t    *ui,
                         const overlayaz_t *o)
{
    GtkWidget *content_area;
    overlayaz_dialog_prefs_t p;
    gint grid_pos;
    GtkCellRenderer *renderer;

    p.o = o;
    p.map_update = FALSE;
    p.map_source_change = FALSE;
    p.dialog = gtk_dialog_new_with_buttons("Preferences",
                                           overlayaz_ui_get_parent(ui),
                                           GTK_DIALOG_MODAL,
                                           "_Cancel",
                                           GTK_RESPONSE_NONE,
                                           "_Apply",
                                           GTK_RESPONSE_APPLY,
                                           NULL);
#ifdef G_OS_WIN32
    g_signal_connect(p.dialog, "realize", G_CALLBACK(mingw_realize), NULL);
#endif

    gtk_window_set_resizable(GTK_WINDOW(p.dialog), FALSE);

    p.grid = gtk_grid_new();
    gtk_widget_set_margin_top(p.grid, OVERLAYAZ_WINDOW_DIALOG_MARGIN);
    gtk_widget_set_margin_start(p.grid, OVERLAYAZ_WINDOW_DIALOG_MARGIN);
    gtk_widget_set_margin_bottom(p.grid, OVERLAYAZ_WINDOW_DIALOG_MARGIN);
    gtk_widget_set_margin_end(p.grid, OVERLAYAZ_WINDOW_DIALOG_MARGIN);
    gtk_grid_set_row_spacing(GTK_GRID(p.grid), OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_grid_set_column_spacing(GTK_GRID(p.grid), 4*OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_grid_set_row_homogeneous(GTK_GRID(p.grid), TRUE);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(p.dialog));
    gtk_box_pack_start(GTK_BOX(content_area), p.grid, FALSE, FALSE, 0);

    grid_pos = 0;

    p.label_srtm = gtk_label_new("SRTM directory:");
    gtk_widget_set_halign(GTK_WIDGET(p.label_srtm), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID (p.grid), p.label_srtm, 1, grid_pos, 1, 1);

    p.file_chooser_srtm = gtk_file_chooser_button_new("SRTM directory", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    gtk_grid_attach(GTK_GRID (p.grid), p.file_chooser_srtm, 2, grid_pos, 1, 1);

    p.label_map_source = gtk_label_new("Map source:");
    gtk_widget_set_halign(GTK_WIDGET(p.label_map_source), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID (p.grid), p.label_map_source, 1, ++grid_pos, 1, 1);

    p.combo_map_source = gtk_combo_box_new_with_model(ui_preferences_map_sources());
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(p.combo_map_source), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(p.combo_map_source), renderer, "text", 0, NULL);
    gtk_cell_layout_set_cell_data_func(GTK_CELL_LAYOUT(p.combo_map_source), renderer, ui_preferences_map_source_render, NULL, NULL);
    gtk_grid_attach(GTK_GRID (p.grid), p.combo_map_source, 2, grid_pos, 1, 1);

    p.label_map_grid_distance = gtk_label_new("Map grid [km]:");
    gtk_widget_set_halign(GTK_WIDGET(p.label_map_grid_distance), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID (p.grid), p.label_map_grid_distance, 1, ++grid_pos, 1, 1);

    p.spin_map_grid_distance = gtk_spin_button_new_with_range(10.0, 1000.0, 1.0);
    gtk_grid_attach(GTK_GRID (p.grid), p.spin_map_grid_distance, 2, grid_pos, 1, 1);

    p.label_location = gtk_label_new("Home location:");
    gtk_widget_set_halign(GTK_WIDGET(p.label_location), GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID (p.grid), p.label_location, 1, ++grid_pos, 1, 1);

    p.button_location = gtk_button_new_from_icon_name("go-home", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_button_set_always_show_image(GTK_BUTTON(p.button_location), TRUE);
    gtk_button_set_label(GTK_BUTTON(p.button_location), "Copy current");
    gtk_grid_attach(GTK_GRID (p.grid), p.button_location, 2, grid_pos, 1, 1);

    p.label_latitude = gtk_label_new("Latitude [°]:");
    gtk_widget_set_halign(GTK_WIDGET(p.label_latitude), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID (p.grid), p.label_latitude, 1, ++grid_pos, 1, 1);

    p.spin_latitude = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_LAT_MIN, OVERLAYAZ_WINDOW_LAT_MAX, OVERLAYAZ_WINDOW_LAT_STEP);
    gtk_grid_attach(GTK_GRID (p.grid), p.spin_latitude, 2, grid_pos, 1, 1);

    p.label_longitude = gtk_label_new("Longitude [°]:");
    gtk_widget_set_halign(GTK_WIDGET(p.label_longitude), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID (p.grid), p.label_longitude, 1, ++grid_pos, 1, 1);

    p.spin_longitude = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_LON_MIN, OVERLAYAZ_WINDOW_LON_MAX, OVERLAYAZ_WINDOW_LON_STEP);
    gtk_grid_attach(GTK_GRID (p.grid), p.spin_longitude, 2, grid_pos, 1, 1);

    p.label_altitude = gtk_label_new("Altitude [m]:");
    gtk_widget_set_halign(GTK_WIDGET(p.label_altitude), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID (p.grid), p.label_altitude, 1, ++grid_pos, 1, 1);

    p.spin_altitude = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_ALT_MIN, OVERLAYAZ_WINDOW_ALT_MAX, OVERLAYAZ_WINDOW_ALT_STEP);
    gtk_grid_attach(GTK_GRID (p.grid), p.spin_altitude, 2, grid_pos, 1, 1);

    p.check_dark_theme = gtk_check_button_new_with_label("Prefer dark theme (restart required)");
    gtk_widget_set_halign(GTK_WIDGET(p.check_dark_theme), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID (p.grid), p.check_dark_theme, 1, ++grid_pos, 2, 1);

    gtk_widget_set_sensitive(p.button_location, overlayaz_get_location(o, NULL));

    ui_preferences_from_config(&p);
    gtk_widget_show_all(p.dialog);

    g_signal_connect(p.combo_map_source, "changed", G_CALLBACK(ui_preferences_combo_map_source_changed), &p);
    g_signal_connect(p.spin_map_grid_distance, "value-changed", G_CALLBACK(ui_preferences_spin_map_grid_distance_changed), &p);
    g_signal_connect(p.button_location, "clicked", G_CALLBACK(ui_preferences_button_location_clicked), &p);

    if (gtk_dialog_run(GTK_DIALOG(p.dialog)) == GTK_RESPONSE_APPLY)
        ui_preferences_to_config(&p);

    if (p.map_source_change)
        overlayaz_ui_set_map_source(ui, gtk_combo_box_get_active(GTK_COMBO_BOX(p.combo_map_source)));

    if (p.map_update)
        overlayaz_ui_update_view(ui, OVERLAYAZ_UI_UPDATE_MAP);

    gtk_widget_destroy(p.dialog);
}

static GtkTreeModel*
ui_preferences_map_sources(void)
{
    GtkListStore *store;
    GtkTreeIter iter;
    gint i;

    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_BOOLEAN);
    for (i = 0; i < OSM_GPS_MAP_SOURCE_LAST; i++)
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                           0, osm_gps_map_source_get_friendly_name(i),
                           1, GPOINTER_TO_INT(osm_gps_map_source_get_repo_uri(i)),
                           -1);
    }

    return GTK_TREE_MODEL(store);
}

static void
ui_preferences_map_source_render(GtkCellLayout   *cell_layout,
                                 GtkCellRenderer *cell,
                                 GtkTreeModel    *tree_model,
                                 GtkTreeIter     *iter,
                                 gpointer         data)
{
    gboolean available;
    gtk_tree_model_get (tree_model, iter, 1, &available, -1);
    g_object_set(cell, "sensitive", available, NULL);
}

static void
ui_preferences_from_config(overlayaz_dialog_prefs_t *p)
{
    gchar *srtm_path;

    srtm_path = overlayaz_conf_get_srtm_path();
    if (srtm_path && strlen(srtm_path))
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(p->file_chooser_srtm), srtm_path);

    gtk_combo_box_set_active(GTK_COMBO_BOX(p->combo_map_source), overlayaz_conf_get_map_source());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(p->spin_map_grid_distance), overlayaz_conf_get_map_grid_distance());

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(p->spin_latitude), overlayaz_conf_get_latitude());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(p->spin_longitude), overlayaz_conf_get_longitude());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(p->spin_altitude), overlayaz_conf_get_altitude());

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(p->check_dark_theme), overlayaz_conf_get_dark_theme());

    g_free(srtm_path);
}

static void
ui_preferences_to_config(overlayaz_dialog_prefs_t *p)
{
    gchar *srtm_path;

    srtm_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(p->file_chooser_srtm));
    if (srtm_path && strlen(srtm_path))
        overlayaz_conf_set_srtm_path(srtm_path);

    overlayaz_conf_set_map_source(gtk_combo_box_get_active(GTK_COMBO_BOX(p->combo_map_source)));
    overlayaz_conf_set_map_grid_distance(gtk_spin_button_get_value(GTK_SPIN_BUTTON(p->spin_map_grid_distance)));

    overlayaz_conf_set_latitude(gtk_spin_button_get_value(GTK_SPIN_BUTTON(p->spin_latitude)));
    overlayaz_conf_set_longitude(gtk_spin_button_get_value(GTK_SPIN_BUTTON(p->spin_longitude)));
    overlayaz_conf_set_altitude(gtk_spin_button_get_value(GTK_SPIN_BUTTON(p->spin_altitude)));

    overlayaz_conf_set_dark_theme(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(p->check_dark_theme)));

    g_free(srtm_path);
}

static void
ui_preferences_button_location_clicked(GtkButton *button,
                                       gpointer   user_data)
{
    overlayaz_dialog_prefs_t *p = (overlayaz_dialog_prefs_t*)user_data;
    struct overlayaz_location location;

    if (overlayaz_get_location(p->o, &location))
    {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(p->spin_latitude), location.latitude);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(p->spin_longitude), location.longitude);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(p->spin_altitude), location.altitude);
    }
}

static void
ui_preferences_combo_map_source_changed(GtkComboBox *combo_box,
                                        gpointer     user_data)
{
    overlayaz_dialog_prefs_t *prefs = (overlayaz_dialog_prefs_t*)user_data;
    prefs->map_source_change = TRUE;
}

static void
ui_preferences_spin_map_grid_distance_changed(GtkSpinButton *spin_button,
                                              gpointer       user_data)
{
    overlayaz_dialog_prefs_t *prefs = (overlayaz_dialog_prefs_t*)user_data;
    prefs->map_update = TRUE;
}
