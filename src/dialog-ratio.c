/*
 *  overlayaz – photo visibility analysis software
 *  Copyright (c) 2023  Konrad Kosmatka
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
#include "dialog-ratio.h"
#include "ui.h"
#include "exif.h"
#include "util.h"
#ifdef G_OS_WIN32
#include "mingw.h"
#endif

#define DIALOG_RATIO_PIXEL_SIZE_MIN  0.0
#define DIALOG_RATIO_PIXEL_SIZE_MAX  100000.0
#define DIALOG_RATIO_PIXEL_SIZE_STEP 1.0

#define DIALOG_RATIO_FOCAL_RES_MIN  0.0
#define DIALOG_RATIO_FOCAL_RES_MAX  100000.0
#define DIALOG_RATIO_FOCAL_RES_STEP 0.0001

#define DIALOG_RATIO_SENSOR_LEN_MIN  0.0
#define DIALOG_RATIO_SENSOR_LEN_MAX  100000.0
#define DIALOG_RATIO_SENSOR_LEN_STEP 0.00001

#define DIALOG_RATIO_FOCAL_LEN_MIN 0.0
#define DIALOG_RATIO_FOCAL_LEN_MAX 10000.0
#define DIALOG_RATIO_FOCAL_LEN_STEP 0.1

struct overlayaz_dialog_ratio
{
    GtkWidget *dialog;
    GtkWidget *label_exif;
    GtkWidget *grid;
    GtkWidget *label_focal_res;
    GtkWidget *label_sensor_len;
    GtkWidget *label_pixel_size;
    GtkWidget *label_focal_len;
    GtkWidget *label_fov;
    GtkWidget *label_ratio;
    GtkWidget *spin_pixel_size;
    GtkWidget *spin_focal_res;
    GtkWidget *spin_sensor_len;
    GtkWidget *spin_focal_len;
    GtkWidget *spin_fov;
    GtkWidget *spin_ratio;
};

static void dialog_ratio_spin_changed(GtkSpinButton*, struct overlayaz_dialog_ratio*);
static void dialog_ratio_spin_focal_res_changed(GtkSpinButton*, struct overlayaz_dialog_ratio*);
static void dialog_ratio_spin_sensor_len_changed(GtkSpinButton*, struct overlayaz_dialog_ratio*);
static gboolean dialog_ratio_spin_generic_format(GtkSpinButton*, gpointer);


gboolean
overlayaz_dialog_ratio(GtkWindow               *parent,
                       const gchar             *filename,
                       enum overlayaz_ref_type  type,
                       gdouble                 *out)
{
    struct overlayaz_dialog_ratio d;
    GtkWidget *content_area;
    gint grid_pos;
    overlayaz_exif_t *exif;
    gdouble x_res, y_res;
    gdouble focal_length;
    gint width, height;
    gboolean focal_res_found;
    gboolean focal_length_found;
    gchar *markup;
    gboolean ret = FALSE;

    d.dialog = gtk_dialog_new_with_buttons("Ratio calculator",
                                           parent,
                                           GTK_DIALOG_MODAL,
                                           "_Cancel",
                                           GTK_RESPONSE_CANCEL,
                                           "_Apply",
                                           GTK_RESPONSE_APPLY,
                                           NULL);
#ifdef G_OS_WIN32
    g_signal_connect(d.dialog, "realize", G_CALLBACK(mingw_realize), NULL);
#endif

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(d.dialog));
    d.label_exif = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(content_area), d.label_exif, FALSE, FALSE, OVERLAYAZ_WINDOW_DIALOG_MARGIN);

    d.grid = gtk_grid_new();
    gtk_widget_set_margin_start(d.grid, OVERLAYAZ_WINDOW_DIALOG_MARGIN);
    gtk_widget_set_margin_bottom(d.grid, OVERLAYAZ_WINDOW_DIALOG_MARGIN);
    gtk_widget_set_margin_end(d.grid, OVERLAYAZ_WINDOW_DIALOG_MARGIN);
    gtk_grid_set_row_spacing(GTK_GRID(d.grid), OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_grid_set_column_spacing(GTK_GRID(d.grid), 2*OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_box_pack_start(GTK_BOX(content_area), d.grid, FALSE, FALSE, 0);

    grid_pos = 0;

    d.label_pixel_size = gtk_label_new((type == OVERLAYAZ_REF_AZ) ? "Sensor width [px]:" : "Sensor height [px]:");
    gtk_widget_set_halign(GTK_WIDGET(d.label_pixel_size), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(d.grid), d.label_pixel_size, 1, grid_pos, 1, 1);

    d.spin_pixel_size = gtk_spin_button_new_with_range(DIALOG_RATIO_PIXEL_SIZE_MIN, DIALOG_RATIO_PIXEL_SIZE_MAX, DIALOG_RATIO_PIXEL_SIZE_STEP);
    gtk_grid_attach(GTK_GRID(d.grid), d.spin_pixel_size, 2, grid_pos, 1, 1);

    d.label_focal_res = gtk_label_new("Focal plane resolution [px/mm]:");
    gtk_widget_set_halign(GTK_WIDGET(d.label_focal_res), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(d.grid), d.label_focal_res, 1, ++grid_pos, 1, 1);

    d.spin_focal_res = gtk_spin_button_new_with_range(DIALOG_RATIO_FOCAL_RES_MIN, DIALOG_RATIO_FOCAL_RES_MAX, DIALOG_RATIO_FOCAL_RES_STEP);
    gtk_grid_attach(GTK_GRID(d.grid), d.spin_focal_res, 2, grid_pos, 1, 1);

    d.label_sensor_len = gtk_label_new((type == OVERLAYAZ_REF_AZ) ? "Sensor width [mm]:" : "Sensor height [mm]:");
    gtk_widget_set_halign(GTK_WIDGET(d.label_sensor_len), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(d.grid), d.label_sensor_len, 1, ++grid_pos, 1, 1);

    d.spin_sensor_len = gtk_spin_button_new_with_range(DIALOG_RATIO_SENSOR_LEN_MIN, DIALOG_RATIO_SENSOR_LEN_MAX, DIALOG_RATIO_SENSOR_LEN_STEP);
    gtk_grid_attach(GTK_GRID(d.grid), d.spin_sensor_len, 2, grid_pos, 1, 1);

    d.label_focal_len = gtk_label_new("Focal length [mm]:");
    gtk_widget_set_halign(GTK_WIDGET(d.label_focal_len), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(d.grid), d.label_focal_len, 1, ++grid_pos, 1, 1);

    d.spin_focal_len = gtk_spin_button_new_with_range(DIALOG_RATIO_FOCAL_LEN_MIN, DIALOG_RATIO_FOCAL_LEN_MAX, DIALOG_RATIO_FOCAL_LEN_STEP);
    gtk_grid_attach(GTK_GRID(d.grid), d.spin_focal_len, 2, grid_pos, 1, 1);

    d.label_fov = gtk_label_new((type == OVERLAYAZ_REF_AZ) ? "Horizontal FOV [°]:" : "Vertical FOV [°]:");
    gtk_widget_set_halign(GTK_WIDGET(d.label_fov), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(d.grid), d.label_fov, 1, ++grid_pos, 1, 1);

    d.spin_fov = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_RATIO_MIN, OVERLAYAZ_WINDOW_RATIO_MAX, OVERLAYAZ_WINDOW_RATIO_STEP);
    gtk_grid_attach(GTK_GRID(d.grid), d.spin_fov, 2, grid_pos, 1, 1);

    d.label_ratio = gtk_label_new((type == OVERLAYAZ_REF_AZ) ? "Azimuth ratio [px/°]:" : "Elevation ratio [px/°]:");
    gtk_widget_set_halign(GTK_WIDGET(d.label_ratio), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(d.grid), d.label_ratio, 1, ++grid_pos, 1, 1);

    d.spin_ratio = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_RATIO_MIN, OVERLAYAZ_WINDOW_RATIO_MAX, OVERLAYAZ_WINDOW_RATIO_STEP);
    gtk_grid_attach(GTK_GRID(d.grid), d.spin_ratio, 2, grid_pos, 1, 1);

    exif = overlayaz_exif_new(filename);
    if (exif)
    {
        focal_res_found = overlayaz_exif_get_focal_plane_res(exif, &x_res, &y_res);
        if (focal_res_found)
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(d.spin_focal_res), (type == OVERLAYAZ_REF_AZ) ? x_res : y_res);

        focal_length_found = overlayaz_exif_get_focal_length(exif, &focal_length);
        if (focal_length_found)
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(d.spin_focal_len), focal_length);

        overlayaz_exif_get_pixel_size_with_fallback(exif, &width, &height);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(d.spin_pixel_size), (type == OVERLAYAZ_REF_AZ) ? width : height);

        if (!focal_res_found && !focal_length_found)
            markup = g_markup_printf_escaped("EXIF status: <b><span color=\"red\">No focal information found</span></b>");
        else if (!focal_res_found)
            markup = g_markup_printf_escaped("EXIF status: <b><span color=\"red\">No focal plane resolution found</span></b>");
        else if (!focal_length_found)
            markup = g_markup_printf_escaped("EXIF status: <b><span color=\"red\">No focal length found</span></b>");
        else
            markup = g_markup_printf_escaped("EXIF status: <b>Lookup succeeded</b>");

        overlayaz_exif_free(exif);
    }
    else
    {
        markup = g_markup_printf_escaped("EXIF status: <b><span color=\"red\">No metadata found</span></b>");
    }

    gtk_label_set_markup(GTK_LABEL(d.label_exif), markup);
    g_free(markup);

    g_signal_connect(d.spin_pixel_size, "value-changed", G_CALLBACK(dialog_ratio_spin_focal_res_changed), &d);
    g_signal_connect(d.spin_focal_res, "value-changed", G_CALLBACK(dialog_ratio_spin_focal_res_changed), &d);
    g_signal_connect(d.spin_sensor_len, "value-changed", G_CALLBACK(dialog_ratio_spin_sensor_len_changed), &d);
    g_signal_connect(d.spin_focal_len, "value-changed", G_CALLBACK(dialog_ratio_spin_changed), &d);

    g_signal_connect(d.spin_pixel_size, "output", G_CALLBACK(dialog_ratio_spin_generic_format), &d);
    g_signal_connect(d.spin_focal_res, "output", G_CALLBACK(dialog_ratio_spin_generic_format), &d);
    g_signal_connect(d.spin_sensor_len, "output", G_CALLBACK(dialog_ratio_spin_generic_format), &d);
    g_signal_connect(d.spin_focal_len, "output", G_CALLBACK(dialog_ratio_spin_generic_format), &d);
    g_signal_connect(d.spin_fov, "output", G_CALLBACK(dialog_ratio_spin_generic_format), &d);
    g_signal_connect(d.spin_ratio, "output", G_CALLBACK(dialog_ratio_spin_generic_format), &d);

    /* Force calculation update */
    dialog_ratio_spin_focal_res_changed(GTK_SPIN_BUTTON(d.spin_focal_res), &d);

    gtk_widget_set_sensitive(d.spin_fov, FALSE);
    gtk_widget_set_sensitive(d.spin_ratio, FALSE);
    gtk_widget_show_all(d.dialog);

    if (gtk_dialog_run(GTK_DIALOG(d.dialog)) == GTK_RESPONSE_APPLY)
    {
        *out = gtk_spin_button_get_value(GTK_SPIN_BUTTON(d.spin_ratio));
        ret = TRUE;
    }

    gtk_widget_destroy(d.dialog);
    return ret;
}

static void
dialog_ratio_spin_changed(GtkSpinButton                 *spin_button,
                          struct overlayaz_dialog_ratio *d)
{
    gint pixel_size = (gint)gtk_spin_button_get_value(GTK_SPIN_BUTTON(d->spin_pixel_size));
    gdouble focal_res = gtk_spin_button_get_value(GTK_SPIN_BUTTON(d->spin_focal_res));
    gdouble sensor_len = gtk_spin_button_get_value(GTK_SPIN_BUTTON(d->spin_sensor_len));
    gdouble focal_length = gtk_spin_button_get_value(GTK_SPIN_BUTTON(d->spin_focal_len));
    gdouble fov;
    gdouble ratio;
    GtkWidget *button;

    if (pixel_size == 0 ||
        pixel_size == DIALOG_RATIO_PIXEL_SIZE_MAX ||
        focal_res < DIALOG_RATIO_FOCAL_RES_STEP ||
        focal_res > DIALOG_RATIO_FOCAL_RES_MAX - DIALOG_RATIO_FOCAL_RES_STEP ||
        sensor_len < DIALOG_RATIO_SENSOR_LEN_STEP ||
        sensor_len > DIALOG_RATIO_SENSOR_LEN_MAX - DIALOG_RATIO_SENSOR_LEN_STEP ||
        focal_length < DIALOG_RATIO_FOCAL_LEN_STEP ||
        focal_length > DIALOG_RATIO_FOCAL_LEN_MAX - DIALOG_RATIO_FOCAL_LEN_STEP)
    {
        fov = 0.0;
        ratio = 0.0;
    }
    else
    {
        fov = overlayaz_util_fov_calc(sensor_len, focal_length);
        ratio = pixel_size / fov;
    }

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(d->spin_fov), fov);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(d->spin_ratio), ratio);
    button = gtk_dialog_get_widget_for_response(GTK_DIALOG(d->dialog), GTK_RESPONSE_APPLY);
    gtk_widget_set_sensitive(button, ratio != 0.0);
}

static void
dialog_ratio_spin_focal_res_changed(GtkSpinButton                 *spin_button,
                                    struct overlayaz_dialog_ratio *d)
{
    gdouble pixel_size = gtk_spin_button_get_value(GTK_SPIN_BUTTON(d->spin_pixel_size));
    gdouble focal_res = gtk_spin_button_get_value(GTK_SPIN_BUTTON(d->spin_focal_res));
    gdouble sensor_len;

    if (pixel_size == 0 ||
        pixel_size == DIALOG_RATIO_PIXEL_SIZE_MAX ||
        focal_res < DIALOG_RATIO_FOCAL_RES_STEP ||
        focal_res > DIALOG_RATIO_FOCAL_RES_MAX - DIALOG_RATIO_FOCAL_RES_STEP)
    {
        sensor_len = 0.0;
    }
    else
    {
        sensor_len = pixel_size / focal_res;
    }

    g_signal_handlers_block_by_func(G_OBJECT(d->spin_sensor_len), GINT_TO_POINTER(dialog_ratio_spin_sensor_len_changed), d);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(d->spin_sensor_len), sensor_len);
    g_signal_handlers_unblock_by_func(G_OBJECT(d->spin_sensor_len), GINT_TO_POINTER(dialog_ratio_spin_sensor_len_changed), d);
    dialog_ratio_spin_changed(spin_button, d);
}

static void
dialog_ratio_spin_sensor_len_changed(GtkSpinButton                 *spin_button,
                                     struct overlayaz_dialog_ratio *d)
{
    gdouble pixel_size = gtk_spin_button_get_value(GTK_SPIN_BUTTON(d->spin_pixel_size));
    gdouble sensor_len = gtk_spin_button_get_value(GTK_SPIN_BUTTON(d->spin_sensor_len));
    gdouble focal_res;

    if (pixel_size == 0 ||
        pixel_size == DIALOG_RATIO_PIXEL_SIZE_MAX ||
        sensor_len < DIALOG_RATIO_SENSOR_LEN_STEP ||
        sensor_len > DIALOG_RATIO_SENSOR_LEN_MAX - DIALOG_RATIO_SENSOR_LEN_STEP)
    {
        focal_res = 0.0;
    }
    else
    {
        focal_res = pixel_size / sensor_len;
    }

    g_signal_handlers_block_by_func(G_OBJECT(d->spin_focal_res), GINT_TO_POINTER(dialog_ratio_spin_focal_res_changed), d);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(d->spin_focal_res), focal_res);
    g_signal_handlers_unblock_by_func(G_OBJECT(d->spin_focal_res), GINT_TO_POINTER(dialog_ratio_spin_focal_res_changed), d);
    dialog_ratio_spin_changed(spin_button, d);
}

static gboolean
dialog_ratio_spin_generic_format(GtkSpinButton *spin_button,
                                 gpointer       user_data)
{
    GtkAdjustment *adj = gtk_spin_button_get_adjustment(spin_button);
    gdouble lower = gtk_adjustment_get_lower(adj);
    gdouble upper = gtk_adjustment_get_upper(adj);
    gdouble step = gtk_adjustment_get_step_increment(adj);
    gdouble value = gtk_adjustment_get_value(adj);

    if (value < lower + step ||
        value > upper - step)
    {
        gtk_entry_set_text(GTK_ENTRY(spin_button), "");
        return TRUE;
    }

    return FALSE;
}
