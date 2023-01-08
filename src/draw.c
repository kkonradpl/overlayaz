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
#include <math.h>
#include "draw.h"
#include "overlayaz.h"
#include "geo.h"
#include "marker-iter.h"
#include "util.h"
#include "ui-util.h"

static void draw_grid(cairo_t*, const overlayaz_t*, enum overlayaz_ref_type);
static void draw_markers(cairo_t*, const overlayaz_t*);
gchar* format_marker_text(const overlayaz_marker_t*, gdouble, gdouble);


void
overlayaz_draw(cairo_t           *cr,
               cairo_filter_t     filter,
               cairo_surface_t   *cache,
               const overlayaz_t *o)
{
    const GdkPixbuf *pixbuf;
    gint width, height;

    pixbuf = overlayaz_get_pixbuf(o);
    if (pixbuf == NULL)
        return;

    width = overlayaz_get_width(o);
    height = overlayaz_get_height(o);

    cairo_save(cr);
    cairo_translate(cr, width/2.0,  height/2.0);
    cairo_rotate(cr, overlayaz_get_rotation(o) * G_PI / 180.0);

    if (cache)
        cairo_set_source_surface(cr, cache, -width/2.0, -height/2.0);
    else
        gdk_cairo_set_source_pixbuf(cr, pixbuf, -width/2.0, -height/2.0);

    cairo_pattern_set_filter(cairo_get_source(cr), filter);
    cairo_paint(cr);
    cairo_restore(cr);

    draw_grid(cr, o, OVERLAYAZ_REF_AZ);
    draw_grid(cr, o, OVERLAYAZ_REF_EL);
    draw_markers(cr, o);
}

static void
draw_grid(cairo_t                 *cr,
          const overlayaz_t       *o,
          enum overlayaz_ref_type  type)
{
    gdouble first;
    gdouble step;
    gint count;
    gint width, height;
    PangoLayout *layout;
    gint i;
    gdouble angle;
    gdouble pos;
    gchar buff[G_ASCII_DTOSTR_BUF_SIZE];
    gchar *text;
    gint layout_width, layout_height;
    gdouble text_width, text_height;
    gdouble offset;
    gdouble x, y;
    gdouble text_pos;

    if (!overlayaz_util_grid_calc(o, type, &first, &step, &count))
        return;

    width = overlayaz_get_width(o);
    height = overlayaz_get_height(o);

    layout = pango_cairo_create_layout(cr);
    pango_layout_set_font_description(layout, overlayaz_font_get_pango(overlayaz_get_grid_font(o)));

    text_pos = 0.0;
    for (i = 0; i < count; i++)
    {
        angle = (type == OVERLAYAZ_REF_AZ ? first + step * i : first - step * i);
        if (!overlayaz_get_position(o, type, angle, &pos))
            continue;

        gdk_cairo_set_source_rgba(cr, overlayaz_get_grid_color(o));
        cairo_set_line_width(cr, overlayaz_get_grid_width(o));

        if (type == OVERLAYAZ_REF_AZ)
        {
            cairo_move_to(cr, pos, 0);
            cairo_line_to(cr, pos, height);
        }
        else
        {
            cairo_move_to(cr, 0, pos);
            cairo_line_to(cr, width, pos);
        }
        cairo_stroke(cr);

        gdk_cairo_set_source_rgba(cr, overlayaz_get_grid_font_color(o));
        g_ascii_formatd(buff, sizeof(buff), "%g", fmod(angle, 360.0));
        text = g_strdup_printf("%s°", buff);
        pango_layout_set_text(layout, text, -1);
        pango_layout_get_size(layout, &layout_width, &layout_height);
        text_width = (gdouble)layout_width / PANGO_SCALE;
        text_height = (gdouble)layout_height / PANGO_SCALE;

        /* The first label (as well as the last one) can exceed the image boundaries. */
        if (text_pos == 0.0)
            text_pos = (type == OVERLAYAZ_REF_AZ ? -text_width : -text_height);

        offset = overlayaz_get_grid_position(o, type) / 100.0;
        if (type == OVERLAYAZ_REF_AZ)
        {
            if (pos - text_width - text_pos >= 0.0)
            {
                x = pos - text_width / 2.0;
                y = offset * (height - text_height);
                cairo_move_to(cr, x, y);
                pango_cairo_show_layout(cr, layout);
                text_pos = x + text_width;
            }
        }
        else
        {
            if (pos - text_height - text_pos >= 0.0)
            {
                x = offset * (width - text_width);
                y = pos - text_height / 2.0;
                cairo_move_to(cr, x, y);
                pango_cairo_show_layout(cr, layout);
                text_pos = y + text_height;
            }
        }
        g_free(text);
    }
    g_object_unref(layout);
}

static void
draw_markers(cairo_t           *cr,
             const overlayaz_t *o)
{
    overlayaz_marker_iter_t *iter;
    struct overlayaz_location home;
    gint height;
    PangoLayout *layout;
    gchar *text;
    gdouble pos, dist, angle;
    gint layout_width, layout_height;
    gdouble lh, x, y;
    const overlayaz_marker_t *m;

    if (!overlayaz_get_location(o, &home))
        return;

    iter = overlayaz_marker_iter_new(overlayaz_get_marker_list(o), &m);
    if (iter == NULL)
        return;

    height = overlayaz_get_height(o);
    layout = pango_cairo_create_layout(cr);
    pango_layout_set_alignment(layout, PANGO_ALIGN_CENTER);

    do
    {
        if (overlayaz_marker_get_active(m))
        {
            overlayaz_geo_inverse(home.latitude, home.longitude,
                                  overlayaz_marker_get_latitude(m),
                                  overlayaz_marker_get_longitude(m),
                                  &angle, NULL, &dist);

            if (overlayaz_get_position(o, OVERLAYAZ_REF_AZ, angle, &pos))
            {
                pango_layout_set_font_description(layout, overlayaz_font_get_pango(overlayaz_marker_get_font(m)));
                gdk_cairo_set_source_rgba(cr, overlayaz_marker_get_font_color(m));
                text = format_marker_text(m, angle, dist);
                if (strlen(text))
                {
                    pango_layout_set_text(layout, text, -1);
                    pango_layout_get_size(layout, &layout_width, &layout_height);
                    lh = (gdouble)layout_height / PANGO_SCALE;

                    x = pos - (gdouble)layout_width / PANGO_SCALE / 2.0;
                    y = overlayaz_marker_get_position(m)/100.0 * height;

                    if (overlayaz_marker_get_tick(m) == OVERLAYAZ_MARKER_TICK_NONE)
                        y -= lh/2;
                    else if (overlayaz_marker_get_tick(m) == OVERLAYAZ_MARKER_TICK_BOTTOM)
                        y -= lh;

                    /* Keep text visible within image bounds */
                    if (y < 0)
                        y = 0;
                    else if (y + lh > height)
                        y = height - lh;

                    cairo_move_to(cr, x, y);
                    pango_cairo_show_layout(cr, layout);
                }
                g_free(text);
            }
        }
    } while (overlayaz_marker_iter_next(iter, &m));

    overlayaz_marker_iter_free(iter);
    g_object_unref(layout);
}

gchar*
format_marker_text(const overlayaz_marker_t *m,
                   gdouble                   angle,
                   gdouble                   dist)
{
    GString *string = g_string_new(NULL);
    gchar *text;

    if (overlayaz_marker_get_tick(m) == OVERLAYAZ_MARKER_TICK_TOP)
        g_string_append(string, "|\n");

    if (strlen(overlayaz_marker_get_name(m)))
    {
        g_string_append(string, overlayaz_marker_get_name(m));
        if (overlayaz_marker_get_show_azimuth(m) ||
            overlayaz_marker_get_show_distance(m))
        {
            g_string_append(string, "\n");
        }
    }

    if (overlayaz_marker_get_show_azimuth(m))
    {
        text = overlayaz_ui_util_format_angle(angle);
        g_string_append(string, text);
        g_free(text);

        if (overlayaz_marker_get_show_distance(m))
            g_string_append(string, "\n");
    }

    if (overlayaz_marker_get_show_distance(m))
    {
        text = overlayaz_ui_util_format_distance(dist);
        g_string_append(string, text);
        g_free(text);
    }

    if (overlayaz_marker_get_tick(m) == OVERLAYAZ_MARKER_TICK_BOTTOM)
        g_string_append(string, "\n|");

    return g_string_free(string, FALSE);
}
