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
#include "dialog-alt.h"
#include "ui.h"
#include "srtm.h"
#include "conf.h"
#ifdef G_OS_WIN32
#include "mingw.h"
#endif

struct overlayaz_dialog_alt
{
    GtkWidget *dialog;
    GtkWidget *label_srtm;
    GtkWidget *grid;
    GtkWidget *label_asl;
    GtkWidget *label_agl;
    GtkWidget *label_total;
    GtkWidget *spin_asl;
    GtkWidget *spin_agl;
    GtkWidget *spin_total;
};

static void dialog_alt_spin_changed(GtkSpinButton*, gpointer);


gboolean
overlayaz_dialog_alt(GtkWindow *parent,
                     gdouble    latitude,
                     gdouble    longitude,
                     gdouble   *out)
{
    GtkWidget *content_area;
    struct overlayaz_dialog_alt d;
    gint grid_pos;
    enum overlayaz_srtm_error error;
    gdouble hasl;
    gchar *directory;
    gchar *filename;
    gchar *markup;
    gboolean ret = FALSE;

    d.dialog = gtk_dialog_new_with_buttons("Altitude",
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

    gtk_window_set_title(GTK_WINDOW(d.dialog), "Altitude");

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(d.dialog));
    d.label_srtm = gtk_label_new(NULL);
    gtk_box_pack_start(GTK_BOX(content_area), d.label_srtm, FALSE, FALSE, OVERLAYAZ_WINDOW_DIALOG_MARGIN);

    d.grid = gtk_grid_new();
    gtk_widget_set_margin_start(d.grid, OVERLAYAZ_WINDOW_DIALOG_MARGIN);
    gtk_widget_set_margin_bottom(d.grid, OVERLAYAZ_WINDOW_DIALOG_MARGIN);
    gtk_widget_set_margin_end(d.grid, OVERLAYAZ_WINDOW_DIALOG_MARGIN);
    gtk_grid_set_row_spacing(GTK_GRID(d.grid), OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_grid_set_column_spacing(GTK_GRID(d.grid), 2*OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_box_pack_start(GTK_BOX(content_area), d.grid, FALSE, FALSE, 0);

    grid_pos = 0;

    d.label_asl = gtk_label_new("Height above sea level [m]:");
    gtk_widget_set_halign(GTK_WIDGET(d.label_asl), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(d.grid), d.label_asl, 1, grid_pos, 1, 1);

    d.spin_asl = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_ALT_MIN, OVERLAYAZ_WINDOW_ALT_MAX, OVERLAYAZ_WINDOW_ALT_STEP);
    gtk_grid_attach(GTK_GRID(d.grid), d.spin_asl, 2, grid_pos, 1, 1);

    d.label_agl = gtk_label_new("Height above ground level [m]:");
    gtk_widget_set_halign(GTK_WIDGET(d.label_agl), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(d.grid), d.label_agl, 1, ++grid_pos, 1, 1);

    d.spin_agl = gtk_spin_button_new_with_range(0, OVERLAYAZ_WINDOW_ALT_MAX, OVERLAYAZ_WINDOW_ALT_STEP);
    gtk_grid_attach(GTK_GRID(d.grid), d.spin_agl, 2, grid_pos, 1, 1);

    d.label_total = gtk_label_new("Total height [m]:");
    gtk_widget_set_halign(GTK_WIDGET(d.label_total), GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(d.grid), d.label_total, 1, ++grid_pos, 1, 1);

    d.spin_total = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_ALT_MIN, OVERLAYAZ_WINDOW_ALT_MAX, OVERLAYAZ_WINDOW_ALT_STEP);
    gtk_grid_attach(GTK_GRID(d.grid), d.spin_total, 2, grid_pos, 1, 1);

    g_signal_connect(d.spin_asl, "value-changed", G_CALLBACK(dialog_alt_spin_changed), &d);
    g_signal_connect(d.spin_agl, "value-changed", G_CALLBACK(dialog_alt_spin_changed), &d);

    gtk_widget_set_sensitive(d.spin_total, FALSE);
    gtk_widget_show_all(d.dialog);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(d.spin_asl), 0.0);
    gtk_widget_grab_focus(d.spin_asl);

    directory = overlayaz_conf_get_srtm_path();
    filename = overlayaz_srtm_filename(latitude, longitude);
    error = overlayaz_srtm_lookup(directory,
                                  latitude,
                                  longitude,
                                  &hasl);

    switch (error)
    {
    case OVERLAYAZ_SRTM_OK:
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(d.spin_asl), hasl);
        gtk_widget_grab_focus(d.spin_agl);
        markup = g_markup_printf_escaped("SRTM status: <b>Lookup succeeded</b>");
        break;

    case OVERLAYAZ_SRTM_ERROR_DIR:
        markup = g_markup_printf_escaped("SRTM status: <b><span color=\"red\">Unable to open directory</span></b>");
        break;

    case OVERLAYAZ_SRTM_ERROR_MISSING:
        markup = g_markup_printf_escaped("SRTM status: <b><span color=\"red\">Missing file (%s)</span></b>", filename);
        break;

    case OVERLAYAZ_SRTM_ERROR_FORMAT:
        markup = g_markup_printf_escaped("SRTM status: <b><span color=\"red\">Unsupported format (%s)</span></b>", filename);
        break;

    case OVERLAYAZ_SRTM_ERROR_OPEN:
        markup = g_markup_printf_escaped("SRTM status: <b><span color=\"red\">Unable to open (%s)</span></b>", filename);
        break;

    case OVERLAYAZ_SRTM_ERROR_READ:
        markup = g_markup_printf_escaped("SRTM status: <b><span color=\"red\">Read failed (%s)</span></b>", filename);
        break;

    case OVERLAYAZ_SRTM_ERROR_INVALID:
        markup = g_markup_printf_escaped("SRTM status: <b><span color=\"red\">Missing value</span></b>");
        break;

    default:
        markup = g_markup_printf_escaped("SRTM status: <b><span color=\"red\">Unknown error</span></b>");
        break;
    }

    g_free(filename);
    g_free(directory);

    if (markup)
    {
        gtk_label_set_markup(GTK_LABEL(d.label_srtm), markup);
        g_free(markup);
    }

    if (gtk_dialog_run(GTK_DIALOG(d.dialog)) == GTK_RESPONSE_APPLY)
    {
        *out = gtk_spin_button_get_value(GTK_SPIN_BUTTON(d.spin_total));
        ret = TRUE;
    }

    gtk_widget_destroy(d.dialog);
    return ret;
}

static void
dialog_alt_spin_changed(GtkSpinButton *spin_button,
                        gpointer       user_data)
{
    struct overlayaz_dialog_alt *a = (struct overlayaz_dialog_alt*)user_data;
    gdouble total;

    total = gtk_spin_button_get_value(GTK_SPIN_BUTTON(a->spin_asl));
    total += gtk_spin_button_get_value(GTK_SPIN_BUTTON(a->spin_agl));

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(a->spin_total), total);
}
