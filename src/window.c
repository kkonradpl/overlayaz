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
#include <osmgpsmap-1.0/osm-gps-map.h>
#include "window.h"
#include "conf.h"

#define WINDOW_MIN_WIDTH 1000


void
overlayaz_window_init(struct overlayaz_window *w)
{
    GtkFileFilter *filter;
    GtkWidget *separator;

    w->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(w->window), OVERLAYAZ_NAME);
    gtk_window_set_icon_name(GTK_WINDOW(w->window), OVERLAYAZ_ICON);
    gtk_window_set_position(GTK_WINDOW(w->window), GTK_WIN_POS_CENTER);
    gtk_widget_add_events(w->window, GDK_KEY_PRESS_MASK);
    gtk_container_set_border_width(GTK_CONTAINER(w->window), 2);

    w->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_container_add(GTK_CONTAINER(w->window), w->box);

    w->box_menu = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    gtk_box_pack_start(GTK_BOX(w->box), w->box_menu, FALSE, FALSE, 0);

    w->file_chooser = gtk_file_chooser_button_new("Open a file", GTK_FILE_CHOOSER_ACTION_OPEN);
    filter = gtk_file_filter_new();
    gtk_file_filter_add_pixbuf_formats(filter);
    gtk_file_filter_add_pattern(filter, "*" OVERLAYAZ_EXTENSION_PROFILE);
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(w->file_chooser), filter);
    gtk_box_pack_start(GTK_BOX(w->box_menu), w->file_chooser, FALSE, FALSE, 0);

    w->notebook_menu = gtk_notebook_new();
    gtk_widget_set_hexpand(w->box_menu, FALSE);
    gtk_box_pack_start(GTK_BOX(w->box_menu), w->notebook_menu, TRUE, TRUE, 0);

    gtk_notebook_append_page(GTK_NOTEBOOK(w->notebook_menu), overlayaz_menu_ref(&w->r), gtk_label_new("Reference"));
    gtk_notebook_append_page(GTK_NOTEBOOK(w->notebook_menu), overlayaz_menu_grid(&w->g), gtk_label_new("Grid"));
    gtk_notebook_append_page(GTK_NOTEBOOK(w->notebook_menu), overlayaz_menu_marker(&w->m), gtk_label_new("Marker"));

    w->grid_meas = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(w->grid_meas), OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_grid_set_column_spacing(GTK_GRID(w->grid_meas), OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_grid_set_column_homogeneous(GTK_GRID(w->grid_meas), TRUE);
    gtk_box_pack_start(GTK_BOX(w->box_menu), w->grid_meas, FALSE, FALSE, 2);

    w->label_meas_first = gtk_label_new("Azimuth:");
    gtk_grid_attach(GTK_GRID(w->grid_meas), w->label_meas_first, 0, 0, 1, 1);

    w->label_meas_second = gtk_label_new("Elevation:");
    gtk_grid_attach(GTK_GRID(w->grid_meas), w->label_meas_second, 1, 0, 1, 1);

    w->label_meas_first_value = gtk_label_new(NULL);
    gtk_grid_attach(GTK_GRID(w->grid_meas), w->label_meas_first_value, 0, 1, 1, 1);

    w->label_meas_second_value = gtk_label_new(NULL);
    gtk_grid_attach(GTK_GRID(w->grid_meas), w->label_meas_second_value, 1, 1, 1, 1);

    w->box_bottom = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_box_pack_start(GTK_BOX(w->box_menu), w->box_bottom, FALSE, FALSE, 0);

    w->button_save = gtk_button_new_from_icon_name("document-save", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_button_set_always_show_image(GTK_BUTTON(w->button_save), TRUE);
    gtk_button_set_label(GTK_BUTTON(w->button_save), "Save profile");
    gtk_box_pack_start(GTK_BOX(w->box_bottom), w->button_save, TRUE, TRUE, 0);

    w->button_preferences = gtk_button_new_from_icon_name("preferences-system-symbolic", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_widget_set_tooltip_text(w->button_preferences, "Preferences");
    gtk_box_pack_start(GTK_BOX(w->box_bottom), w->button_preferences, FALSE, FALSE, 0);

    w->button_about = gtk_button_new_from_icon_name("help-about-symbolic", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_widget_set_tooltip_text(w->button_about, "About overlayaz");
    gtk_box_pack_start(GTK_BOX(w->box_bottom), w->button_about, FALSE, FALSE, 0);

    w->button_export = gtk_button_new_from_icon_name("document-save-as", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_button_set_always_show_image(GTK_BUTTON(w->button_export), TRUE);
    gtk_button_set_label(GTK_BUTTON(w->button_export), "Export image");
    gtk_box_pack_start(GTK_BOX(w->box_bottom), w->button_export, TRUE, TRUE, 0);

    separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(w->box), separator, FALSE, FALSE, 0);

    /* View */
    w->notebook_view = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(w->box), w->notebook_view, TRUE, TRUE, 0);

    w->box_image = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_notebook_append_page(GTK_NOTEBOOK(w->notebook_view), w->box_image, gtk_label_new("Image"));

    w->image = gtk_drawing_area_new();
    g_object_set(w->image, "can-focus", TRUE, NULL);
    gtk_widget_add_events(w->image, GDK_BUTTON_PRESS_MASK |
                                           GDK_BUTTON_RELEASE_MASK |
                                           GDK_POINTER_MOTION_MASK |
                                           GDK_LEAVE_NOTIFY_MASK |
                                           GDK_SCROLL_MASK);

    gtk_box_pack_start(GTK_BOX(w->box_image), w->image, TRUE, TRUE, 0);

    separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(w->box_image), separator, FALSE, FALSE, 0);

    w->frame_rotation = gtk_frame_new(NULL);
    gtk_box_pack_start(GTK_BOX(w->box_image), w->frame_rotation, FALSE, FALSE, 0);

    w->box_rotation = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_container_add(GTK_CONTAINER(w->frame_rotation), w->box_rotation);

    w->label_rotation = gtk_label_new("Rotate");
    gtk_box_pack_start(GTK_BOX(w->box_rotation), w->label_rotation, FALSE, FALSE, 0);

    w->scale_rotation = gtk_scale_new_with_range(GTK_ORIENTATION_VERTICAL, OVERLAYAZ_WINDOW_ROTATION_MIN, OVERLAYAZ_WINDOW_ROTATION_MAX, OVERLAYAZ_WINDOW_ROTATION_STEP);
    gtk_scale_set_has_origin(GTK_SCALE(w->scale_rotation), FALSE);
    gtk_scale_set_value_pos(GTK_SCALE(w->scale_rotation), GTK_POS_BOTTOM);
    gtk_range_set_value(GTK_RANGE(w->scale_rotation), 0.0);
    gtk_box_pack_start(GTK_BOX(w->box_rotation), w->scale_rotation, TRUE, TRUE, 0);

    w->button_rotation_reset = gtk_button_new_with_label("R");
    gtk_widget_set_tooltip_text(w->button_rotation_reset, "Reset rotation");
    gtk_box_pack_start(GTK_BOX(w->box_rotation), w->button_rotation_reset, FALSE, FALSE, 0);

    w->box_map = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_notebook_append_page(GTK_NOTEBOOK(w->notebook_view), w->box_map, gtk_label_new("Map"));

    /* The map-source must be passed in an object constructor. Otherwise, it is ignored in older osm-gps-map (1.1.0).
     * This will produce "Map source setup called twice" critical error (1.2.0), but it seems to be a library bug. */
    w->map = g_object_new(OSM_TYPE_GPS_MAP,
                          "map-source", overlayaz_conf_get_map_source(),
                          "tile-cache", OSM_GPS_MAP_CACHE_AUTO,
                          "proxy-uri", g_getenv("http_proxy"),
                          NULL);
    gtk_widget_add_events(w->map, GDK_LEAVE_NOTIFY_MASK);
    gtk_box_pack_start(GTK_BOX(w->box_map),GTK_WIDGET(w->map), TRUE, TRUE, 0);

    gtk_widget_set_size_request(w->window, WINDOW_MIN_WIDTH, -1);
    gtk_widget_show_all(w->window);
}
