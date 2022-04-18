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

#define PRECISION_DISTANCE_BASE 4
#define PRECISION_ANGLE         2


static gchar* format_value_and_unit(gdouble, gint, const gchar*);


void
overlayaz_ui_util_set_spin_button_text_visibility(GtkSpinButton *button,
                                                  gboolean       visible)
{
    GtkAdjustment *adj;

    if (visible)
    {
        /* HACK: Reattach the GtkAdjustment to revert the displayed value */
        adj = gtk_spin_button_get_adjustment(button);
        if (adj)
            gtk_spin_button_set_adjustment(button, adj);
    }
    else
    {
        gtk_entry_set_text(GTK_ENTRY(button), "");
    }
}

gboolean
overlayaz_ui_util_format_spin_button_zero(GtkSpinButton *widget,
                                          gpointer       user_data)
{
    GtkAdjustment *adj = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(widget));
    if (gtk_adjustment_get_value(adj) == 0.0)
    {
        gtk_entry_set_text(GTK_ENTRY(widget), "");
        return GDK_EVENT_STOP;
    }
    return GDK_EVENT_PROPAGATE;
}

gchar*
overlayaz_ui_util_format_distance(gdouble value)
{
    gint precision;

    if (value < 1000)
        return format_value_and_unit(value, 0, " m");

    value /= 1000.0;
    precision = PRECISION_DISTANCE_BASE - (gint)ceil(log10(value));
    if (precision < 0)
        precision = 0;

    return format_value_and_unit(value, precision, " km");
}

gchar*
overlayaz_ui_util_format_angle(gdouble value)
{
    return format_value_and_unit(value, PRECISION_ANGLE, "°");
}

static gchar*
format_value_and_unit(gdouble      value,
                      gint         precision,
                      const gchar *unit)
{
    if (isnan(value))
        return g_strdup(" ");
    else
        return g_strdup_printf("%.*f%s", precision, value, unit);
}

