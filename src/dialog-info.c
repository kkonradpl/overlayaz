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
#include "ui.h"
#include "ui-util.h"
#include "conf.h"
#include "srtm.h"

struct overlayaz_dialog_info
{
    GtkWidget *dialog;
    GtkWidget *grid;
    GtkWidget *label_x;
    GtkWidget *label_x_value;
    GtkWidget *label_y;
    GtkWidget *label_y_value;
    GtkWidget *label_latitude;
    GtkWidget *label_latitude_value;
    GtkWidget *label_longitude;
    GtkWidget *label_longitude_value;
    GtkWidget *label_altitude;
    GtkWidget *label_altitude_value;
    GtkWidget *label_azimuth;
    GtkWidget *label_azimuth_value;
    GtkWidget *label_elevation;
    GtkWidget *label_elevation_value;
    GtkWidget *label_distance;
    GtkWidget *label_distance_value;
    GtkWidget *button_x;
    GtkWidget *button_y;
    GtkWidget *button_latitude;
    GtkWidget *button_longitude;
    GtkWidget *button_altitude;
    GtkWidget *button_azimuth;
    GtkWidget *button_elevation;
    GtkWidget *button_distance;
    GtkWidget *button_location;
    GtkWidget *button_azidist;
};

static void dialog_info_copy_clicked(GtkButton*, GtkLabel*);
static void dialog_info_copy_location_clicked(GtkButton *button, struct overlayaz_dialog_info*);
static void dialog_info_copy_azidist_clicked(GtkButton*, struct overlayaz_dialog_info*);


void
overlayaz_dialog_info(GtkWindow *parent,
                      gdouble    pos_x,
                      gdouble    pos_y,
                      gdouble    latitude,
                      gdouble    longitude,
                      gdouble    azimuth,
                      gdouble    elevation,
                      gdouble    distance)
{
    struct overlayaz_dialog_info *d = g_malloc0(sizeof(struct overlayaz_dialog_info));
    GtkWidget *content_area;
    enum overlayaz_srtm_error error;
    gchar *text, *text2, *directory, *filename;
    gdouble altitude;
    gint grid_pos = -1;
    gint pos_latitude = -1;
    gint pos_azimuth = -1;

    if (isnan(pos_x) &&
        isnan(pos_y) &&
        isnan(latitude) &&
        isnan(longitude) &&
        isnan(azimuth) &&
        isnan(elevation) &&
        isnan(distance))
    {
        /* Nothing to show here */
        return;
    }

    d->dialog = gtk_dialog_new_with_buttons("Info",
                                           parent,
                                           GTK_DIALOG_MODAL,
                                           "_Close",
                                           GTK_RESPONSE_NONE,
                                           NULL);

    d->grid = gtk_grid_new();
    gtk_widget_set_margin_top(d->grid, OVERLAYAZ_WINDOW_DIALOG_MARGIN);
    gtk_widget_set_margin_start(d->grid, OVERLAYAZ_WINDOW_DIALOG_MARGIN);
    gtk_widget_set_margin_bottom(d->grid, OVERLAYAZ_WINDOW_DIALOG_MARGIN);
    gtk_widget_set_margin_end(d->grid, OVERLAYAZ_WINDOW_DIALOG_MARGIN);
    gtk_grid_set_row_spacing(GTK_GRID(d->grid), OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_grid_set_column_spacing(GTK_GRID(d->grid), 3*OVERLAYAZ_WINDOW_GRID_SPACING);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(d->dialog));
    gtk_box_pack_start(GTK_BOX(content_area), d->grid, FALSE, FALSE, 0);

    if (!isnan(pos_x))
    {
        d->label_x = gtk_label_new("X:");
        gtk_widget_set_halign(GTK_WIDGET(d->label_x), GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(d->grid), d->label_x, 1, ++grid_pos, 1, 1);

        d->label_x_value = gtk_label_new(NULL);
        gtk_grid_attach(GTK_GRID(d->grid), d->label_x_value, 2, grid_pos, 1, 1);

        text = g_strdup_printf("<b>%.1f px</b>", pos_x);
        gtk_label_set_markup(GTK_LABEL(d->label_x_value), text);
        g_free(text);

        d->button_x = gtk_button_new_from_icon_name("edit-copy", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
        gtk_grid_attach(GTK_GRID(d->grid), d->button_x, 3, grid_pos, 1, 1);
        g_signal_connect(d->button_x, "clicked", G_CALLBACK(dialog_info_copy_clicked), d->label_x_value);
    }

    if (!isnan(pos_y))
    {
        d->label_y = gtk_label_new("Y:");
        gtk_widget_set_halign(GTK_WIDGET(d->label_y), GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(d->grid), d->label_y, 1, ++grid_pos, 1, 1);

        d->label_y_value = gtk_label_new(NULL);
        gtk_grid_attach(GTK_GRID(d->grid), d->label_y_value, 2, grid_pos, 1, 1);

        text = g_strdup_printf("<b>%.1f px</b>", pos_y);
        gtk_label_set_markup(GTK_LABEL(d->label_y_value), text);
        g_free(text);

        d->button_y = gtk_button_new_from_icon_name("edit-copy", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
        gtk_grid_attach(GTK_GRID(d->grid), d->button_y, 3, grid_pos, 1, 1);
        g_signal_connect(d->button_y, "clicked", G_CALLBACK(dialog_info_copy_clicked), d->label_y_value);
    }

    if (!isnan(latitude))
    {
        d->label_latitude = gtk_label_new("Latitude:");
        gtk_widget_set_halign(GTK_WIDGET(d->label_latitude), GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(d->grid), d->label_latitude, 1, ++grid_pos, 1, 1);

        d->label_latitude_value = gtk_label_new(NULL);
        gtk_grid_attach(GTK_GRID(d->grid), d->label_latitude_value, 2, grid_pos, 1, 1);

        text = g_strdup_printf("<b>%.6f°</b>", latitude);
        gtk_label_set_markup(GTK_LABEL(d->label_latitude_value), text);
        g_free(text);

        d->button_latitude = gtk_button_new_from_icon_name("edit-copy", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
        gtk_grid_attach(GTK_GRID(d->grid), d->button_latitude, 3, grid_pos, 1, 1);
        g_signal_connect(d->button_latitude, "clicked", G_CALLBACK(dialog_info_copy_clicked), d->label_latitude_value);

        pos_latitude = grid_pos;
    }

    if (!isnan(longitude))
    {
        d->label_longitude = gtk_label_new("Longitude:");
        gtk_widget_set_halign(GTK_WIDGET(d->label_longitude), GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(d->grid), d->label_longitude, 1, ++grid_pos, 1, 1);

        d->label_longitude_value = gtk_label_new(NULL);
        gtk_grid_attach(GTK_GRID(d->grid), d->label_longitude_value, 2, grid_pos, 1, 1);

        text = g_strdup_printf("<b>%.6f°</b>", longitude);
        gtk_label_set_markup(GTK_LABEL(d->label_longitude_value), text);
        g_free(text);

        d->button_longitude = gtk_button_new_from_icon_name("edit-copy", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
        gtk_grid_attach(GTK_GRID(d->grid), d->button_longitude, 3, grid_pos, 1, 1);
        g_signal_connect(d->button_longitude, "clicked", G_CALLBACK(dialog_info_copy_clicked), d->label_longitude_value);
    }

    if(!isnan(latitude) &&
       !isnan(longitude))
    {
        directory = overlayaz_conf_get_srtm_path();
        filename = overlayaz_srtm_filename(latitude, longitude);
        error = overlayaz_srtm_lookup(directory,
                                      latitude,
                                      longitude,
                                      &altitude);

        if (error == OVERLAYAZ_SRTM_OK)
        {
            d->label_altitude = gtk_label_new("Altitude:");
            gtk_widget_set_halign(GTK_WIDGET(d->label_altitude), GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(d->grid), d->label_altitude, 1, ++grid_pos, 1, 1);

            d->label_altitude_value = gtk_label_new(NULL);
            gtk_grid_attach(GTK_GRID(d->grid), d->label_altitude_value, 2, grid_pos, 1, 1);

            text = g_strdup_printf("<b>%g m</b>", altitude);
            gtk_label_set_markup(GTK_LABEL(d->label_altitude_value), text);
            g_free(text);

            d->button_altitude = gtk_button_new_from_icon_name("edit-copy", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
            gtk_grid_attach(GTK_GRID(d->grid), d->button_altitude, 3, grid_pos, 1, 1);
            g_signal_connect(d->button_altitude, "clicked", G_CALLBACK(dialog_info_copy_clicked), d->label_altitude_value);
        }
        g_free(filename);
        g_free(directory);
    }

    if (!isnan(azimuth))
    {
        d->label_azimuth = gtk_label_new("Azimuth:");
        gtk_widget_set_halign(GTK_WIDGET(d->label_azimuth), GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(d->grid), d->label_azimuth, 1, ++grid_pos, 1, 1);

        d->label_azimuth_value = gtk_label_new(NULL);
        gtk_grid_attach(GTK_GRID(d->grid), d->label_azimuth_value, 2, grid_pos, 1, 1);

        text = overlayaz_ui_util_format_angle(azimuth);
        text2 = g_strdup_printf("<b>%s</b>", text);
        gtk_label_set_markup(GTK_LABEL(d->label_azimuth_value), text2);
        g_free(text2);
        g_free(text);

        d->button_azimuth = gtk_button_new_from_icon_name("edit-copy", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
        gtk_grid_attach(GTK_GRID(d->grid), d->button_azimuth, 3, grid_pos, 1, 1);
        g_signal_connect(d->button_azimuth, "clicked", G_CALLBACK(dialog_info_copy_clicked), d->label_azimuth_value);

        pos_azimuth = grid_pos;
    }

    if (!isnan(elevation))
    {
        d->label_elevation = gtk_label_new("Elevation:");
        gtk_widget_set_halign(GTK_WIDGET(d->label_elevation), GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(d->grid), d->label_elevation, 1, ++grid_pos, 1, 1);

        d->label_elevation_value = gtk_label_new(NULL);
        gtk_grid_attach(GTK_GRID(d->grid), d->label_elevation_value, 2, grid_pos, 1, 1);

        text = overlayaz_ui_util_format_angle(elevation);
        text2 = g_strdup_printf("<b>%s</b>", text);
        gtk_label_set_markup(GTK_LABEL(d->label_elevation_value), text2);
        g_free(text2);
        g_free(text);

        d->button_elevation = gtk_button_new_from_icon_name("edit-copy", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
        gtk_grid_attach(GTK_GRID(d->grid), d->button_elevation, 3, grid_pos, 1, 1);
        g_signal_connect(d->button_elevation, "clicked", G_CALLBACK(dialog_info_copy_clicked), d->label_elevation_value);
    }

    if (!isnan(distance))
    {
        d->label_distance = gtk_label_new("Distance:");
        gtk_widget_set_halign(GTK_WIDGET(d->label_distance), GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(d->grid), d->label_distance, 1, ++grid_pos, 1, 1);

        d->label_distance_value = gtk_label_new(NULL);
        gtk_grid_attach(GTK_GRID(d->grid), d->label_distance_value, 2, grid_pos, 1, 1);

        text = overlayaz_ui_util_format_distance(distance);
        text2 = g_strdup_printf("<b>%s</b>", text);
        gtk_label_set_markup(GTK_LABEL(d->label_distance_value), text2);
        g_free(text2);
        g_free(text);

        d->button_distance = gtk_button_new_from_icon_name("edit-copy", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
        gtk_grid_attach(GTK_GRID(d->grid), d->button_distance, 3, grid_pos, 1, 1);
        g_signal_connect(d->button_distance, "clicked", G_CALLBACK(dialog_info_copy_clicked), d->label_distance_value);
    }

    if (d->label_latitude_value &&
        d->label_longitude_value)
    {
        d->button_location = gtk_button_new_from_icon_name("edit-copy", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
        gtk_grid_attach(GTK_GRID(d->grid), d->button_location, 4, pos_latitude, 1, 2 + (d->label_altitude_value != NULL));

        g_signal_connect(d->button_location, "clicked", G_CALLBACK(dialog_info_copy_location_clicked), d);
    }

    if (d->label_azimuth_value &&
        d->label_distance_value)
    {
        d->button_azidist = gtk_button_new_from_icon_name("edit-copy", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
        gtk_grid_attach(GTK_GRID(d->grid), d->button_azidist, 4, pos_azimuth, 1, 2);
        g_signal_connect(d->button_azidist, "clicked", G_CALLBACK(dialog_info_copy_azidist_clicked), d);
    }

    gtk_widget_show_all(d->dialog);
    gtk_widget_grab_focus(gtk_dialog_get_widget_for_response(GTK_DIALOG(d->dialog), GTK_RESPONSE_NONE));
    gtk_dialog_run(GTK_DIALOG(d->dialog));
    gtk_widget_destroy(d->dialog);
    
    g_free(d);
}


static void
dialog_info_copy_clicked(GtkButton *button,
                         GtkLabel  *label)
{
    GtkClipboard *clipboard = gtk_widget_get_clipboard(GTK_WIDGET(button), GDK_SELECTION_CLIPBOARD);
    if (clipboard)
        gtk_clipboard_set_text(clipboard, gtk_label_get_text(label), -1);
}

static void
dialog_info_copy_location_clicked(GtkButton                    *button,
                                  struct overlayaz_dialog_info *d)
{
    GtkClipboard *clipboard = gtk_widget_get_clipboard(GTK_WIDGET(button), GDK_SELECTION_CLIPBOARD);
    gchar *text;

    if (clipboard)
    {
        text = g_strdup_printf("%s %s%s%s",
                               gtk_label_get_text(GTK_LABEL(d->label_latitude_value)),
                               gtk_label_get_text(GTK_LABEL(d->label_longitude_value)),
                               (d->label_altitude_value ? " " : ""),
                               (d->label_altitude_value ? gtk_label_get_text(GTK_LABEL(d->label_altitude_value)) : ""));
        gtk_clipboard_set_text(clipboard, text, -1);
        g_free(text);
    }
}

static void
dialog_info_copy_azidist_clicked(GtkButton                    *button,
                                 struct overlayaz_dialog_info *d)
{
    GtkClipboard *clipboard = gtk_widget_get_clipboard(GTK_WIDGET(button), GDK_SELECTION_CLIPBOARD);
    gchar *text;

    if (clipboard)
    {
        text = g_strdup_printf("%s %s",
                               gtk_label_get_text(GTK_LABEL(d->label_azimuth_value)),
                               gtk_label_get_text(GTK_LABEL(d->label_distance_value)));
        gtk_clipboard_set_text(clipboard, text, -1);
        g_free(text);
    }
}
