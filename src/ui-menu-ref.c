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
#include "ui-menu-ref.h"
#include "dialog-alt.h"
#include "conf.h"
#include "icon.h"
#include "ui-util.h"
#include "exif.h"
#include "dialog.h"

struct overlayaz_ui_menu_ref
{
    overlayaz_ui_t *ui;
    struct overlayaz_menu_ref *r;
    overlayaz_t *o;
    gboolean lock;
    GtkButton *ref;
    GtkWidget *image_picker;
    GtkWidget *image_home;
    GtkWidget *image_ref[OVERLAYAZ_REF_TYPES][OVERLAYAZ_REF_IDS];
};

static void ui_menu_ref_sync(overlayaz_ui_menu_ref_t*, enum overlayaz_ref_type);
static void ui_menu_ref_update(overlayaz_ui_menu_ref_t*, enum overlayaz_ref_type);

static void ui_menu_ref_spin_location_changed(GtkSpinButton*, overlayaz_ui_menu_ref_t*);
static void ui_menu_ref_combo_mode_changed(GtkComboBox*, overlayaz_ui_menu_ref_t*);
static void ui_menu_ref_button_ref_clicked(GtkButton*, overlayaz_ui_menu_ref_t*);
static void ui_menu_ref_button_alt_clicked(GtkButton*, overlayaz_ui_menu_ref_t*);
static void ui_menu_ref_button_home_clicked(GtkButton*, overlayaz_ui_menu_ref_t*);
static void ui_menu_ref_button_exif_clicked(GtkButton*, overlayaz_ui_menu_ref_t*);
static void ui_menu_ref_spin_ref_changed(GtkSpinButton*, overlayaz_ui_menu_ref_t*);
static void ui_menu_ref_button_ratio_clicked(GtkButton*, overlayaz_ui_menu_ref_t*);
static void ui_menu_ref_spin_ratio_changed(GtkSpinButton*, overlayaz_ui_menu_ref_t*);

static gboolean overlayaz_ui_menu_ref_set_button(overlayaz_ui_menu_ref_t*, GtkButton*);

overlayaz_ui_menu_ref_t*
overlayaz_ui_menu_ref_new(overlayaz_ui_t            *ui,
                          struct overlayaz_menu_ref *r,
                          overlayaz_t               *o)
{
    overlayaz_ui_menu_ref_t *ui_r = g_malloc0(sizeof(overlayaz_ui_menu_ref_t));
    gint width, height;
    gint icon_size;
    gint t, i;

    ui_r->ui = ui;
    ui_r->r = r;
    ui_r->o = o;

    if (gtk_icon_size_lookup(OVERLAYAZ_WINDOW_BUTTON_IMAGE, &width, &height))
        icon_size = MAX(width, height);
    else
        icon_size = 16;

    ui_r->image_picker = gtk_image_new_from_icon_name("gtk-color-picker", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    g_object_ref(ui_r->image_picker);
    ui_r->image_home = gtk_image_new_from_pixbuf(overlayaz_icon_home(icon_size));
    g_object_ref(ui_r->image_home);
    
    g_signal_connect(ui_r->r->spin_latitude, "value-changed", G_CALLBACK(ui_menu_ref_spin_location_changed), ui_r);
    g_signal_connect(ui_r->r->spin_longitude, "value-changed", G_CALLBACK(ui_menu_ref_spin_location_changed), ui_r);
    g_signal_connect(ui_r->r->spin_altitude, "value-changed", G_CALLBACK(ui_menu_ref_spin_location_changed), ui_r);

    for (t = 0; t < OVERLAYAZ_REF_TYPES; t++)
    {
        g_object_set_data(G_OBJECT(ui_r->r->refs[t].combo_mode), "ref-type", GINT_TO_POINTER(t));
        g_signal_connect(ui_r->r->refs[t].combo_mode, "changed", G_CALLBACK(ui_menu_ref_combo_mode_changed), ui_r);
        for (i = 0; i < OVERLAYAZ_REF_IDS; i++)
        {
            g_object_set_data(G_OBJECT(ui_r->r->refs[t].button_ref[i]), "ref-type", GINT_TO_POINTER(t));
            g_object_set_data(G_OBJECT(ui_r->r->refs[t].button_ref[i]), "ref-id", GINT_TO_POINTER(i));
            g_signal_connect(ui_r->r->refs[t].button_ref[i], "clicked", G_CALLBACK(ui_menu_ref_button_ref_clicked), ui_r);

            g_object_set_data(G_OBJECT(ui_r->r->refs[t].button_altitude[i]), "ref-id", GINT_TO_POINTER(i));
            g_signal_connect(ui_r->r->refs[t].button_altitude[i], "clicked", G_CALLBACK(ui_menu_ref_button_alt_clicked), ui_r);

            g_object_set_data(G_OBJECT(ui_r->r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_LATITUDE]), "ref-type", GINT_TO_POINTER(t));
            g_signal_connect(ui_r->r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_LATITUDE], "value-changed", G_CALLBACK(ui_menu_ref_spin_ref_changed), ui_r);

            g_object_set_data(G_OBJECT(ui_r->r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_LONGITUDE]), "ref-type", GINT_TO_POINTER(t));
            g_signal_connect(ui_r->r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_LONGITUDE], "value-changed", G_CALLBACK(ui_menu_ref_spin_ref_changed), ui_r);

            g_object_set_data(G_OBJECT(ui_r->r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_ALTITUDE]), "ref-type", GINT_TO_POINTER(t));
            g_signal_connect(ui_r->r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_ALTITUDE], "value-changed", G_CALLBACK(ui_menu_ref_spin_ref_changed), ui_r);

            g_object_set_data(G_OBJECT(ui_r->r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_POSITION]), "ref-type", GINT_TO_POINTER(t));
            g_signal_connect(ui_r->r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_POSITION], "value-changed", G_CALLBACK(ui_menu_ref_spin_ref_changed), ui_r);

            ui_r->image_ref[t][i] = gtk_image_new_from_pixbuf(overlayaz_icon_ref(icon_size, t, i));
            g_object_ref(ui_r->image_ref[t][i]);

            g_signal_connect(ui_r->r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_ANGLE], "output", G_CALLBACK(overlayaz_ui_util_format_spin_button_zero), NULL);
        }
        g_object_set_data(G_OBJECT(ui_r->r->refs[t].spin_ratio), "ref-type", GINT_TO_POINTER(t));
        g_object_set_data(G_OBJECT(ui_r->r->refs[t].button_ratio_sync), "ref-type", GINT_TO_POINTER(t));
        g_signal_connect(ui_r->r->refs[t].button_ratio_sync, "clicked", G_CALLBACK(ui_menu_ref_button_ratio_clicked), ui_r);
        g_signal_connect(ui_r->r->refs[t].spin_ratio, "value-changed", G_CALLBACK(ui_menu_ref_spin_ratio_changed), ui_r);
        g_signal_connect(ui_r->r->refs[t].spin_ratio, "output", G_CALLBACK(overlayaz_ui_util_format_spin_button_zero), NULL);
    }

    g_object_set_data(G_OBJECT(ui_r->r->button_coord_ref), "ref-type", GINT_TO_POINTER(-1));
    g_object_set_data(G_OBJECT(ui_r->r->button_coord_ref), "ref-id", GINT_TO_POINTER(-1));
    g_signal_connect(ui_r->r->button_coord_ref, "clicked", G_CALLBACK(ui_menu_ref_button_ref_clicked), ui_r);
    g_object_set_data(G_OBJECT(ui_r->r->button_altitude), "ref-id", GINT_TO_POINTER(-1));
    g_signal_connect(ui_r->r->button_altitude, "clicked", G_CALLBACK(ui_menu_ref_button_alt_clicked), ui_r);
    g_signal_connect(ui_r->r->button_home, "clicked", G_CALLBACK(ui_menu_ref_button_home_clicked), ui_r);
    g_signal_connect(ui_r->r->button_exif, "clicked", G_CALLBACK(ui_menu_ref_button_exif_clicked), ui_r);

    return ui_r;
}

void
overlayaz_ui_menu_ref_free(overlayaz_ui_menu_ref_t *ui_r)
{
    gint t, i;

    g_object_unref(ui_r->image_picker);
    g_object_unref(ui_r->image_home);
    for (t = 0; t < OVERLAYAZ_REF_TYPES; t++)
        for (i = 0; i < OVERLAYAZ_REF_IDS; i++)
            g_object_unref(ui_r->image_ref[t][i]);

    g_free(ui_r);
}

void
overlayaz_ui_menu_ref_sync(overlayaz_ui_menu_ref_t *ui_r,
                           gboolean                 active)
{
    struct overlayaz_location location;

    gtk_widget_set_sensitive(ui_r->r->spin_latitude, active);
    gtk_widget_set_sensitive(ui_r->r->button_coord_ref, active);
    gtk_widget_set_sensitive(ui_r->r->spin_longitude, active);
    gtk_widget_set_sensitive(ui_r->r->button_altitude, active);
    gtk_widget_set_sensitive(ui_r->r->button_home, active);
    gtk_widget_set_sensitive(ui_r->r->button_exif, active);
    gtk_widget_set_sensitive(ui_r->r->spin_altitude, active);
    gtk_widget_set_sensitive(ui_r->r->refs[0].combo_mode, active);
    gtk_widget_set_sensitive(ui_r->r->refs[1].combo_mode, active);

    ui_r->lock = TRUE;

    if (!overlayaz_get_location(ui_r->o, &location))
    {
        location.latitude = 0.0;
        location.longitude = 0.0;
        location.altitude = 0.0;
    }

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->spin_latitude), location.latitude);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->spin_longitude), location.longitude);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->spin_altitude), location.altitude);

    ui_menu_ref_sync(ui_r, OVERLAYAZ_REF_AZ);
    ui_menu_ref_sync(ui_r, OVERLAYAZ_REF_EL);

    overlayaz_ui_menu_ref_none(ui_r);

    ui_r->lock = FALSE;
}

void
overlayaz_ui_menu_ref_none(overlayaz_ui_menu_ref_t *ui_r)
{
    overlayaz_ui_menu_ref_set_button(ui_r, NULL);
}

gboolean
overlayaz_ui_menu_ref_get(overlayaz_ui_menu_ref_t *ui_r,
                          enum overlayaz_ref_type *ref_type,
                          enum overlayaz_ref_id   *ref_id)
{
    if (ui_r->ref == NULL)
        return FALSE;

    if (ref_type)
        *ref_type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(ui_r->ref), "ref-type"));

    if (ref_id)
        *ref_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(ui_r->ref), "ref-id"));

    return TRUE;
}

void
overlayaz_ui_menu_ref_set_position(overlayaz_ui_menu_ref_t *ui_r,
                                   gdouble                  x,
                                   gdouble                  y)
{
    gint ref_id, ref_type;
    gdouble value;

    if (ui_r->ref)
    {
        ref_type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(ui_r->ref), "ref-type"));
        ref_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(ui_r->ref), "ref-id"));
        if (ref_type >= 0 && ref_id >= 0)
        {
            value = (ref_type == OVERLAYAZ_REF_AZ) ? x : y;
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[ref_type].spin_ref[ref_id][OVERLAYAZ_REF_RANGE_POSITION]), value);
        }
    }
}

void
overlayaz_ui_menu_ref_set_location(overlayaz_ui_menu_ref_t *ui_r,
                                   gdouble                  lat,
                                   gdouble                  lon)
{
    gint t, i;

    /* Location */
    if (ui_r->ref == GTK_BUTTON(ui_r->r->button_coord_ref))
    {
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->spin_latitude), lat);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->spin_longitude), lon);
        return;
    }

    /* References */
    for (t = 0; t < OVERLAYAZ_REF_TYPES; t++)
    {
        for (i = 0; i < OVERLAYAZ_REF_IDS; i++)
        {
            if (ui_r->ref == GTK_BUTTON(ui_r->r->refs[t].button_ref[i]))
            {
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_LATITUDE]), lat);
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[t].spin_ref[i][OVERLAYAZ_REF_RANGE_LONGITUDE]), lon);
                return;
            }
        }
    }
}

static void
ui_menu_ref_sync(overlayaz_ui_menu_ref_t *ui_r,
                 enum overlayaz_ref_type  type)
{
    struct overlayaz_location location;
    gdouble position;
    gdouble angle;
    gdouble ratio;
    gint limit;
    gboolean ref_pos[OVERLAYAZ_REF_IDS];
    gboolean ref_loc[OVERLAYAZ_REF_IDS];
    gint i;

    limit = (type == OVERLAYAZ_REF_AZ) ? overlayaz_get_width(ui_r->o) : overlayaz_get_height(ui_r->o);
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_POSITION]), 0, limit);
    gtk_spin_button_set_range(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_POSITION]), 0, limit);

    /* HACK: We want to make sure that the "changed" signal is really emitted */
    gtk_combo_box_set_active(GTK_COMBO_BOX(ui_r->r->refs[type].combo_mode), -1);

    for (i = 0; i < OVERLAYAZ_REF_IDS; i++)
    {
        ref_loc[i] = overlayaz_get_ref_location(ui_r->o, type, i, &location);
        if (ref_loc[i])
        {
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[i][OVERLAYAZ_REF_RANGE_LATITUDE]), location.latitude);
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[i][OVERLAYAZ_REF_RANGE_LONGITUDE]), location.longitude);
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[i][OVERLAYAZ_REF_RANGE_ALTITUDE]), location.altitude);
        }
        else
        {
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[i][OVERLAYAZ_REF_RANGE_LATITUDE]), 0.0);
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[i][OVERLAYAZ_REF_RANGE_LONGITUDE]), 0.0);
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[i][OVERLAYAZ_REF_RANGE_ALTITUDE]), 0.0);
        }

        ref_pos[i] = overlayaz_get_ref_position(ui_r->o, type, i, &position);
        if (ref_pos[i])
        {
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[i][OVERLAYAZ_REF_RANGE_POSITION]), position);

            if (overlayaz_get_angle(ui_r->o, type, position, &angle))
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[i][OVERLAYAZ_REF_RANGE_ANGLE]), angle);
            else
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[i][OVERLAYAZ_REF_RANGE_ANGLE]), 0.0);
        }
        else
        {
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[i][OVERLAYAZ_REF_RANGE_POSITION]), 0.0);
        }
    }

    if (overlayaz_get_ratio(ui_r->o, type, &ratio))
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ratio), ratio);
    else
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ratio), 0.0);

    if (ref_loc[OVERLAYAZ_REF_A] || ref_pos[OVERLAYAZ_REF_A])
    {
        if (ref_loc[OVERLAYAZ_REF_B] || ref_pos[OVERLAYAZ_REF_B])
            gtk_combo_box_set_active(GTK_COMBO_BOX(ui_r->r->refs[type].combo_mode), OVERLAYAZ_REF_MODE_TWO_POINT);
        else
            gtk_combo_box_set_active(GTK_COMBO_BOX(ui_r->r->refs[type].combo_mode), OVERLAYAZ_REF_MODE_ONE_POINT);
    }
    else
    {
        gtk_combo_box_set_active(GTK_COMBO_BOX(ui_r->r->refs[type].combo_mode), OVERLAYAZ_REF_MODE_DISABLED);
    }
}

static void
ui_menu_ref_update(overlayaz_ui_menu_ref_t *ui_r,
                   enum overlayaz_ref_type  type)
{
    struct overlayaz_location refA_location;
    struct overlayaz_location refB_location;
    gdouble refA_position, refB_position, ratio, angle;
    gint mode;

    mode = gtk_combo_box_get_active(GTK_COMBO_BOX(ui_r->r->refs[type].combo_mode));
    refA_location.latitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[OVERLAYAZ_REF_A][OVERLAYAZ_REF_RANGE_LATITUDE]));
    refA_location.longitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[OVERLAYAZ_REF_A][OVERLAYAZ_REF_RANGE_LONGITUDE]));
    refA_location.altitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[OVERLAYAZ_REF_A][OVERLAYAZ_REF_RANGE_ALTITUDE]));
    refA_position = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[OVERLAYAZ_REF_A][OVERLAYAZ_REF_RANGE_POSITION]));
    refB_location.latitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[OVERLAYAZ_REF_B][OVERLAYAZ_REF_RANGE_LATITUDE]));
    refB_location.longitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[OVERLAYAZ_REF_B][OVERLAYAZ_REF_RANGE_LONGITUDE]));
    refB_location.altitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[OVERLAYAZ_REF_B][OVERLAYAZ_REF_RANGE_ALTITUDE]));
    refB_position = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[OVERLAYAZ_REF_B][OVERLAYAZ_REF_RANGE_POSITION]));
    ratio = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ratio));

    if (mode == OVERLAYAZ_REF_MODE_ONE_POINT)
    {
        overlayaz_set_ref_one(ui_r->o, type,
                              &refA_location, refA_position,
                              ratio);

        ui_r->lock = TRUE;
        if (!overlayaz_get_angle(ui_r->o, type, refA_position, &angle))
            angle = 0.0;
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[OVERLAYAZ_REF_A][OVERLAYAZ_REF_RANGE_ANGLE]), angle);
        ui_r->lock = FALSE;
    }
    else if (mode == OVERLAYAZ_REF_MODE_TWO_POINT)
    {
        overlayaz_set_ref_two(ui_r->o, type,
                              &refA_location, refA_position,
                              &refB_location, refB_position);

        ui_r->lock = TRUE;
        if (!overlayaz_get_angle(ui_r->o, type, refA_position, &angle))
            angle = 0.0;
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[OVERLAYAZ_REF_A][OVERLAYAZ_REF_RANGE_ANGLE]), angle);
        if (!overlayaz_get_angle(ui_r->o, type, refB_position, &angle))
            angle = 0.0;
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[OVERLAYAZ_REF_B][OVERLAYAZ_REF_RANGE_ANGLE]), angle);
        if (overlayaz_get_ratio(ui_r->o, type, &ratio))
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ratio), ratio);
        else
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ratio), 0.0);
        ui_r->lock = FALSE;
    }
    else
    {
        overlayaz_set_ref_none(ui_r->o, type);
    }

    overlayaz_ui_update_view(ui_r->ui, OVERLAYAZ_UI_UPDATE_IMAGE | OVERLAYAZ_UI_UPDATE_MAP);
}

static void
ui_menu_ref_spin_location_changed(GtkSpinButton           *spin_button,
                                  overlayaz_ui_menu_ref_t *ui_r)
{
    struct overlayaz_location location;

    if (!ui_r->lock)
    {
        location.latitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_r->r->spin_latitude));
        location.longitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_r->r->spin_longitude));
        location.altitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_r->r->spin_altitude));
        overlayaz_set_location(ui_r->o, &location);

        ui_menu_ref_update(ui_r, OVERLAYAZ_REF_AZ);
        ui_menu_ref_update(ui_r, OVERLAYAZ_REF_EL);
    }
}

static void
ui_menu_ref_combo_mode_changed(GtkComboBox             *widget,
                               overlayaz_ui_menu_ref_t *ui_r)
{
    gint mode = gtk_combo_box_get_active(widget);
    gint mode_azimuth = gtk_combo_box_get_active(GTK_COMBO_BOX(ui_r->r->refs[OVERLAYAZ_REF_AZ].combo_mode));
    gint mode_elevation = gtk_combo_box_get_active(GTK_COMBO_BOX(ui_r->r->refs[OVERLAYAZ_REF_EL].combo_mode));
    gint type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "ref-type"));

    if (mode == OVERLAYAZ_REF_MODE_ONE_POINT)
    {
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_ANGLE], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_POSITION], TRUE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_ALTITUDE], TRUE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_ANGLE], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_POSITION], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_ALTITUDE], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].button_altitude[0], TRUE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].button_altitude[1], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].button_ref[0], TRUE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].button_ref[1], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ratio, TRUE);

        /* We want to preserve the input in the spin button, but also to hide it from user */
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ratio), TRUE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_ANGLE]), TRUE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_POSITION]), TRUE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_ALTITUDE]), TRUE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_ANGLE]), FALSE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_POSITION]), FALSE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_ALTITUDE]), FALSE);
    }
    else if (mode == OVERLAYAZ_REF_MODE_TWO_POINT)
    {
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_ANGLE], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_POSITION], TRUE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_ALTITUDE], TRUE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_ANGLE], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_POSITION], TRUE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_ALTITUDE], TRUE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].button_altitude[0], TRUE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].button_altitude[1], TRUE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].button_ref[0], TRUE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].button_ref[1], TRUE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ratio, FALSE);

        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ratio), TRUE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_ANGLE]), TRUE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_POSITION]), TRUE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_ALTITUDE]), TRUE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_ANGLE]), TRUE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_POSITION]), TRUE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_ALTITUDE]), TRUE);
    }
    else if (mode == OVERLAYAZ_REF_MODE_DISABLED)
    {
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_ANGLE], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_POSITION], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_ALTITUDE], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_ANGLE], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_POSITION], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_ALTITUDE], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].button_altitude[0], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].button_altitude[1], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].button_ref[0], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].button_ref[1], FALSE);
        gtk_widget_set_sensitive(ui_r->r->refs[type].spin_ratio, FALSE);

        /* We want to hide the input value from the spin button for user, but keep it internally */
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ratio), FALSE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_ANGLE]), FALSE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_POSITION]), FALSE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[0][OVERLAYAZ_REF_RANGE_ALTITUDE]), FALSE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_ANGLE]), FALSE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_POSITION]), FALSE);
        overlayaz_ui_util_set_spin_button_text_visibility(GTK_SPIN_BUTTON(ui_r->r->refs[type].spin_ref[1][OVERLAYAZ_REF_RANGE_ALTITUDE]), FALSE);
    }
    else
    {
        return;
    }

    if ((mode_azimuth == OVERLAYAZ_REF_MODE_ONE_POINT && mode_elevation != OVERLAYAZ_REF_MODE_DISABLED))
        gtk_widget_set_sensitive(ui_r->r->refs[OVERLAYAZ_REF_AZ].button_ratio_sync, TRUE);
    else
        gtk_widget_set_sensitive(ui_r->r->refs[OVERLAYAZ_REF_AZ].button_ratio_sync, FALSE);

    if (mode_elevation == OVERLAYAZ_REF_MODE_ONE_POINT && mode_azimuth != OVERLAYAZ_REF_MODE_DISABLED)
        gtk_widget_set_sensitive(ui_r->r->refs[OVERLAYAZ_REF_EL].button_ratio_sync, TRUE);
    else
        gtk_widget_set_sensitive(ui_r->r->refs[OVERLAYAZ_REF_EL].button_ratio_sync, FALSE);

    if (!ui_r->lock)
        ui_menu_ref_update(ui_r, type);
}

static void
ui_menu_ref_button_ref_clicked(GtkButton               *button,
                               overlayaz_ui_menu_ref_t *ui_r)
{
    if (overlayaz_ui_menu_ref_set_button(ui_r, button))
    {
        /* Automatically switch view to the map */
        if (button == GTK_BUTTON(ui_r->r->button_coord_ref))
            overlayaz_ui_set_view(ui_r->ui, OVERLAYAZ_WINDOW_VIEW_MAP);
    }

    /* Redraw canvas to highlight the reference which is going to be modified */
    if (button != GTK_BUTTON(ui_r->r->button_coord_ref))
        overlayaz_ui_update_view(ui_r->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
}

static void
ui_menu_ref_button_alt_clicked(GtkButton               *button,
                               overlayaz_ui_menu_ref_t *ui_r)
{
    gint ref_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "ref-id"));
    GtkWidget *widget_latitude;
    GtkWidget *widget_longitude;
    GtkWidget *widget_altitude;
    gdouble latitude;
    gdouble longitude;
    gdouble altitude;

    if (ref_id < 0)
    {
        widget_latitude = ui_r->r->spin_latitude;
        widget_longitude = ui_r->r->spin_longitude;
        widget_altitude = ui_r->r->spin_altitude;
    }
    else
    {
        widget_latitude = ui_r->r->refs[OVERLAYAZ_REF_EL].spin_ref[ref_id][OVERLAYAZ_REF_RANGE_LATITUDE];
        widget_longitude = ui_r->r->refs[OVERLAYAZ_REF_EL].spin_ref[ref_id][OVERLAYAZ_REF_RANGE_LONGITUDE];
        widget_altitude = ui_r->r->refs[OVERLAYAZ_REF_EL].spin_ref[ref_id][OVERLAYAZ_REF_RANGE_ALTITUDE];
    }

    latitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget_latitude));
    longitude = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget_longitude));

    if (overlayaz_dialog_alt(overlayaz_ui_get_parent(ui_r->ui), latitude, longitude, &altitude))
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget_altitude), altitude);
}

static void
ui_menu_ref_button_home_clicked(GtkButton               *button,
                                overlayaz_ui_menu_ref_t *ui_r)
{
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->spin_latitude), overlayaz_conf_get_latitude());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->spin_longitude), overlayaz_conf_get_longitude());
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->spin_altitude), overlayaz_conf_get_altitude());
}

static void
ui_menu_ref_button_exif_clicked(GtkButton               *button,
                                overlayaz_ui_menu_ref_t *ui_r)
{
    const gchar *filename = overlayaz_get_filename(ui_r->o);
    struct overlayaz_location location;

    if (filename == NULL)
        return;

    if (!overlayaz_exif_get_location(filename, &location))
    {
        overlayaz_dialog(overlayaz_ui_get_parent(ui_r->ui),
                         GTK_MESSAGE_WARNING,
                         "EXIF lookup",
                         "Failed to lookup location from EXIF metadata.");
        return;
    }

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->spin_latitude), location.latitude);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->spin_longitude), location.longitude);

    if (location.altitude)
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->spin_altitude), location.altitude);
}

static void
ui_menu_ref_spin_ref_changed(GtkSpinButton           *spin_button,
                             overlayaz_ui_menu_ref_t *ui_r)
{
    gint type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(spin_button), "ref-type"));

    if (!ui_r->lock)
        ui_menu_ref_update(ui_r, type);
}

static void
ui_menu_ref_spin_ratio_changed(GtkSpinButton           *spin_button,
                               overlayaz_ui_menu_ref_t *ui_r)
{
    gint type = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(spin_button), "ref-type"));

    if (!ui_r->lock)
        ui_menu_ref_update(ui_r, type);
}

static void
ui_menu_ref_button_ratio_clicked(GtkButton               *button,
                                 overlayaz_ui_menu_ref_t *ui_r)
{
    gint target = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "ref-type"));
    gint source = (gint) (!target);
    gdouble ratio = gtk_spin_button_get_value(GTK_SPIN_BUTTON(ui_r->r->refs[source].spin_ratio));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_r->r->refs[target].spin_ratio), ratio);
}

static gboolean
overlayaz_ui_menu_ref_set_button(overlayaz_ui_menu_ref_t *ui_r,
                                 GtkButton               *button)
{
    gboolean deselect;
    gint t, i;

    deselect = button && gtk_button_get_image(GTK_BUTTON(button)) == ui_r->image_picker;

    for (t = 0; t < OVERLAYAZ_REF_TYPES; t++)
    {
        for (i = 0; i < OVERLAYAZ_REF_IDS; i++)
        {
            gtk_button_set_image(GTK_BUTTON(ui_r->r->refs[t].button_ref[i]), ui_r->image_ref[t][i]);
            gtk_style_context_remove_class(gtk_widget_get_style_context(ui_r->r->refs[t].button_ref[i]), GTK_STYLE_CLASS_DESTRUCTIVE_ACTION);
            gtk_style_context_add_class(gtk_widget_get_style_context(ui_r->r->refs[t].button_ref[i]), GTK_STYLE_CLASS_SUGGESTED_ACTION);
        }
    }

    gtk_button_set_image(GTK_BUTTON(ui_r->r->button_coord_ref), ui_r->image_home);
    gtk_style_context_remove_class(gtk_widget_get_style_context(ui_r->r->button_coord_ref), GTK_STYLE_CLASS_DESTRUCTIVE_ACTION);
    gtk_style_context_add_class(gtk_widget_get_style_context(ui_r->r->button_coord_ref), GTK_STYLE_CLASS_SUGGESTED_ACTION);

    if (button && !deselect)
    {
        gtk_button_set_image(GTK_BUTTON(button), ui_r->image_picker);
        gtk_style_context_remove_class(gtk_widget_get_style_context(GTK_WIDGET(button)), GTK_STYLE_CLASS_SUGGESTED_ACTION);
        gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(button)), GTK_STYLE_CLASS_DESTRUCTIVE_ACTION);
        ui_r->ref = button;
        return TRUE;
    }

    ui_r->ref = NULL;
    return FALSE;
}
