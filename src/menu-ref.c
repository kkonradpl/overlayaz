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
#include "window.h"
#include "icon.h"


GtkWidget*
overlayaz_menu_ref(struct overlayaz_menu_ref *r)
{
    gint grid_pos = 0;
    gint t, i;

    r->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    r->grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(r->grid), OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_grid_set_column_spacing(GTK_GRID(r->grid), OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_box_pack_start(GTK_BOX(r->box), r->grid, FALSE, FALSE, 0);

    r->label_latitude = gtk_label_new("Latitude [°]:");
    gtk_grid_attach(GTK_GRID(r->grid), r->label_latitude, 0, grid_pos, 1, 1);

    r->label_longitude = gtk_label_new("Longitude [°]:");
    gtk_grid_attach(GTK_GRID(r->grid), r->label_longitude, 2, grid_pos, 1, 1);

    r->spin_latitude = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_LAT_MIN, OVERLAYAZ_WINDOW_LAT_MAX, OVERLAYAZ_WINDOW_LAT_STEP);
    gtk_grid_attach(GTK_GRID(r->grid), r->spin_latitude, 0, ++grid_pos, 1, 1);

    r->button_coord_ref = gtk_button_new();
    gtk_grid_attach(GTK_GRID(r->grid), r->button_coord_ref, 1, grid_pos, 1, 1);

    r->spin_longitude = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_LON_MIN, OVERLAYAZ_WINDOW_LON_MAX, OVERLAYAZ_WINDOW_LON_STEP);
    gtk_grid_attach(GTK_GRID(r->grid), r->spin_longitude, 2, grid_pos, 1, 1);

    r->box_location = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    gtk_grid_attach(GTK_GRID(r->grid), r->box_location, 0, ++grid_pos, 1, 1);

    r->button_home = gtk_button_new_from_icon_name("go-home-symbolic", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_widget_set_tooltip_text(r->button_home, "Use predefined home location");
    gtk_box_pack_start(GTK_BOX(r->box_location), r->button_home, FALSE, FALSE, 0);

    r->button_exif = gtk_button_new_from_icon_name("find-location-symbolic", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_widget_set_tooltip_text(r->button_exif, "Lookup EXIF location");
    gtk_box_pack_start(GTK_BOX(r->box_location), r->button_exif, FALSE, FALSE, 0);

    r->label_altitude = gtk_label_new("Altitude [m]:");
    gtk_widget_set_halign(GTK_WIDGET(r->label_altitude), GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(r->box_location), r->label_altitude, TRUE, TRUE, 0);

    r->button_altitude = gtk_button_new_from_icon_name("gtk-index", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_widget_set_tooltip_text(r->button_altitude, "Lookup altitude level");
    gtk_grid_attach(GTK_GRID(r->grid), r->button_altitude, 1, grid_pos, 1, 1);

    r->spin_altitude = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_ALT_MIN, OVERLAYAZ_WINDOW_ALT_MAX, OVERLAYAZ_WINDOW_ALT_STEP);
    gtk_grid_attach(GTK_GRID(r->grid), r->spin_altitude, 2, grid_pos, 1, 1);

    for (t = 0; t < OVERLAYAZ_REF_TYPES; t++)
    {
        r->refs[t].combo_mode = gtk_combo_box_text_new();
        gtk_widget_set_hexpand(r->refs[t].combo_mode, TRUE);
        if (t == OVERLAYAZ_REF_AZ)
        {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(r->refs[t].combo_mode), "Azimuth: Disabled");
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(r->refs[t].combo_mode), "Azimuth: Single reference with ratio");
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(r->refs[t].combo_mode), "Azimuth: Two-point reference");
        }
        else
        {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(r->refs[t].combo_mode), "Elevation: Disabled");
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(r->refs[t].combo_mode), "Elevation: Single reference with ratio");
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(r->refs[t].combo_mode), "Elevation: Two-point reference");
        }
        gtk_grid_attach(GTK_GRID(r->grid), r->refs[t].combo_mode, 0, ++grid_pos, 3, 1);

        r->refs->label_azimuth = gtk_label_new(t == OVERLAYAZ_REF_AZ ? "Azimuth [°]:" : "Elevation [°]:");
        gtk_grid_attach(GTK_GRID(r->grid), r->refs->label_azimuth, 0, ++grid_pos, 1, 1);

        r->refs->label_position = gtk_label_new(t == OVERLAYAZ_REF_AZ ? "Width [px]:" : "Height [px]:");
        gtk_grid_attach(GTK_GRID(r->grid), r->refs->label_position, 2, grid_pos, 1, 1);

        for (i = 0; i < OVERLAYAZ_REF_IDS; i++)
        {
            /* Latitude and longitude reference spin buttons are used internally. */
            r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_LATITUDE] = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_LAT_MIN, OVERLAYAZ_WINDOW_LAT_MAX, OVERLAYAZ_WINDOW_LAT_STEP);
            gtk_widget_set_no_show_all(r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_LATITUDE], TRUE);
            gtk_grid_attach(GTK_GRID(r->grid), r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_LATITUDE], 0, ++grid_pos, 1, 1);

            r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_LONGITUDE] = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_LON_MIN, OVERLAYAZ_WINDOW_LON_MAX, OVERLAYAZ_WINDOW_LON_STEP);
            gtk_widget_set_no_show_all(r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_LONGITUDE], TRUE);
            gtk_grid_attach(GTK_GRID(r->grid), r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_LONGITUDE], 2, grid_pos, 1, 1);

            r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_ANGLE] = gtk_spin_button_new_with_range((t == OVERLAYAZ_REF_AZ ? OVERLAYAZ_WINDOW_AZI_MIN : -OVERLAYAZ_WINDOW_AZI_MAX), OVERLAYAZ_WINDOW_AZI_MAX, OVERLAYAZ_WINDOW_AZI_STEP);
            gtk_grid_attach(GTK_GRID(r->grid), r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_ANGLE], 0, ++grid_pos, 1, 1);

            r->refs[t].button_ref[i] = gtk_button_new();
            gtk_grid_attach(GTK_GRID(r->grid), r->refs[t].button_ref[i], 1, grid_pos, 1, 1);

            r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_POSITION] = gtk_spin_button_new_with_range(0, 1, 0.1);
            gtk_grid_attach(GTK_GRID(r->grid), r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_POSITION], 2, grid_pos, 1, 1);

            r->refs[t].label_altitude[i] = gtk_label_new("Altitude [m]:");
            gtk_widget_set_halign(GTK_WIDGET(r->refs[t].label_altitude[i]), GTK_ALIGN_END);
            gtk_grid_attach(GTK_GRID(r->grid), r->refs[t].label_altitude[i], 0, ++grid_pos, 1, 1);

            r->refs[t].button_altitude[i] = gtk_button_new_from_icon_name("gtk-index", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
            gtk_widget_set_tooltip_text(r->refs[t].button_altitude[i], "Lookup altitude level from SRTM files");
            gtk_grid_attach(GTK_GRID(r->grid), r->refs[t].button_altitude[i], 1, grid_pos, 1, 1);

            r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_ALTITUDE] = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_ALT_MIN, OVERLAYAZ_WINDOW_ALT_MAX, OVERLAYAZ_WINDOW_ALT_STEP);
            gtk_grid_attach(GTK_GRID(r->grid), r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_ALTITUDE], 2, grid_pos, 1, 1);

            if (t == OVERLAYAZ_REF_EL)
            {
                /* Altitude is needed only for elevation reference. */
                gtk_widget_set_no_show_all(r->refs[OVERLAYAZ_REF_AZ].label_altitude[i], TRUE);
                gtk_widget_set_no_show_all(r->refs[OVERLAYAZ_REF_AZ].button_altitude[i], TRUE);
                gtk_widget_set_no_show_all(r->refs[OVERLAYAZ_REF_AZ].spin_ref[i][OVERLAYAZ_REF_RANGE_ALTITUDE], TRUE);
            }
        }

        r->refs[t].label_ratio = gtk_label_new("Ratio [px/°]:");
        gtk_widget_set_halign(GTK_WIDGET(r->refs[t].label_ratio), GTK_ALIGN_END);
        gtk_grid_attach(GTK_GRID(r->grid), r->refs[t].label_ratio, 0, ++grid_pos, 1, 1);

        r->refs[t].button_ratio_sync = gtk_button_new_from_icon_name("edit-paste", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
        gtk_grid_attach(GTK_GRID(r->grid), r->refs[t].button_ratio_sync, 1, grid_pos, 1, 1);
        if (t == OVERLAYAZ_REF_AZ)
            gtk_widget_set_tooltip_text(r->refs[t].button_ratio_sync, "Paste ratio value from elevation reference");
        else if (t == OVERLAYAZ_REF_EL)
            gtk_widget_set_tooltip_text(r->refs[t].button_ratio_sync, "Paste ratio value from azimuth reference");

        r->refs[t].spin_ratio = gtk_spin_button_new_with_range(0, 10000, 0.001);
        gtk_grid_attach(GTK_GRID(r->grid), r->refs[t].spin_ratio, 2, grid_pos, 1, 1);
    }

    return r->box;
}
