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
#include "marker.h"
#include "font.h"

#define OVERLAYAZ_DEFAULT_MARKER_NAME          "Marker"
#define OVERLAYAZ_DEFAULT_MARKER_LATITUDE      0.0
#define OVERLAYAZ_DEFAULT_MARKER_LONGITUDE     0.0
#define OVERLAYAZ_DEFAULT_MARKER_TICK          OVERLAYAZ_MARKER_TICK_BOTTOM
#define OVERLAYAZ_DEFAULT_MARKER_FONT          "monospace 12"
#define OVERLAYAZ_DEFAULT_MARKER_FONT_COLOR    "rgba(0,0,0,0.9)"
#define OVERLAYAZ_DEFAULT_MARKER_POSITION      50.0
#define OVERLAYAZ_DEFAULT_MARKER_ACTIVE        TRUE
#define OVERLAYAZ_DEFAULT_MARKER_SHOW_AZIMUTH  TRUE
#define OVERLAYAZ_DEFAULT_MARKER_SHOW_DISTANCE TRUE

struct overlayaz_marker
{
    gchar *name;
    gdouble latitude;
    gdouble longitude;
    gint tick;
    overlayaz_font_t *font;
    GdkRGBA font_color;
    gdouble position;
    gboolean active;
    gboolean show_azimuth;
    gboolean show_distance;
};


overlayaz_marker_t*
overlayaz_marker_new()
{
    overlayaz_marker_t *marker = g_malloc0(sizeof(overlayaz_marker_t));
    marker->name = g_strdup(OVERLAYAZ_DEFAULT_MARKER_NAME);
    marker->latitude = OVERLAYAZ_DEFAULT_MARKER_LATITUDE;
    marker->longitude = OVERLAYAZ_DEFAULT_MARKER_LONGITUDE;
    marker->tick = OVERLAYAZ_DEFAULT_MARKER_TICK;
    marker->font = overlayaz_font_new(OVERLAYAZ_DEFAULT_MARKER_FONT);
    gdk_rgba_parse(&marker->font_color, OVERLAYAZ_DEFAULT_MARKER_FONT_COLOR);
    marker->position = OVERLAYAZ_DEFAULT_MARKER_POSITION;
    marker->active = OVERLAYAZ_DEFAULT_MARKER_ACTIVE;
    marker->show_azimuth = OVERLAYAZ_DEFAULT_MARKER_SHOW_AZIMUTH;
    marker->show_distance = OVERLAYAZ_DEFAULT_MARKER_SHOW_DISTANCE;
    return marker;
}

void
overlayaz_marker_free(overlayaz_marker_t *marker)
{
    if (marker)
    {
        g_free(marker->name);
        overlayaz_font_free(marker->font);
        g_free(marker);
    }
}

void
overlayaz_marker_set_name(overlayaz_marker_t *marker,
                          const gchar        *value)
{
    g_free(marker->name);
    marker->name = g_strdup(value);
}

const gchar*
overlayaz_marker_get_name(const overlayaz_marker_t *marker)
{
    return marker->name;
}

void
overlayaz_marker_set_latitude(overlayaz_marker_t *marker,
                              gdouble             value)
{
    marker->latitude = value;
}

gdouble
overlayaz_marker_get_latitude(const overlayaz_marker_t *marker)
{
    return marker->latitude;
}

void
overlayaz_marker_set_longitude(overlayaz_marker_t *marker,
                               gdouble             value)
{
    marker->longitude = value;
}

gdouble
overlayaz_marker_get_longitude(const overlayaz_marker_t *marker)
{
    return marker->longitude;
}

void
overlayaz_marker_set_tick(overlayaz_marker_t         *marker,
                          enum overlayaz_marker_tick  value)
{
    marker->tick = value;
}

enum overlayaz_marker_tick
overlayaz_marker_get_tick(const overlayaz_marker_t *marker)
{
    return marker->tick;
}

void
overlayaz_marker_set_font(overlayaz_marker_t *marker,
                          const gchar        *value)
{
    overlayaz_font_set(marker->font, value);
}

const overlayaz_font_t*
overlayaz_marker_get_font(const overlayaz_marker_t *marker)
{
    return marker->font;
}

void
overlayaz_marker_set_font_color(overlayaz_marker_t *marker,
                                const gchar*        value)
{
    gdk_rgba_parse(&marker->font_color, value);
}

void
overlayaz_marker_set_font_color_rgba(overlayaz_marker_t *marker,
                                     const GdkRGBA      *value)
{
    marker->font_color = *value;
}

const GdkRGBA*
overlayaz_marker_get_font_color(const overlayaz_marker_t *marker)
{
    return &marker->font_color;
}

void
overlayaz_marker_set_position(overlayaz_marker_t *marker,
                              gdouble             value)
{
    marker->position = value;
}

gdouble
overlayaz_marker_get_position(const overlayaz_marker_t *marker)
{
    return marker->position;
}

void
overlayaz_marker_set_active(overlayaz_marker_t *marker,
                            gboolean            value)
{
    marker->active = value;
}

gboolean
overlayaz_marker_get_active(const overlayaz_marker_t *marker)
{
    return marker->active;
}

void
overlayaz_marker_set_show_azimuth(overlayaz_marker_t *marker,
                                  gboolean            value)
{
    marker->show_azimuth = value;
}

gboolean
overlayaz_marker_get_show_azimuth(const overlayaz_marker_t *marker)
{
    return marker->show_azimuth;
}

void
overlayaz_marker_set_show_distance(overlayaz_marker_t *marker,
                                   gboolean            value)
{
    marker->show_distance = value;
}

gboolean
overlayaz_marker_get_show_distance(const overlayaz_marker_t *marker)
{
    return marker->show_distance;
}
