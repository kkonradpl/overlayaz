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

#ifndef OVERLAYAZ_MARKER_H_
#define OVERLAYAZ_MARKER_H_

#include "font.h"

typedef struct overlayaz_marker overlayaz_marker_t;

enum overlayaz_marker_tick
{
    OVERLAYAZ_MARKER_TICK_NONE = 0,
    OVERLAYAZ_MARKER_TICK_TOP = 1,
    OVERLAYAZ_MARKER_TICK_BOTTOM = 2
};

overlayaz_marker_t* overlayaz_marker_new();
void overlayaz_marker_free(overlayaz_marker_t*);

void overlayaz_marker_set_name(overlayaz_marker_t*, const gchar*);
const gchar* overlayaz_marker_get_name(const overlayaz_marker_t*);

void overlayaz_marker_set_latitude(overlayaz_marker_t*, gdouble);
gdouble overlayaz_marker_get_latitude(const overlayaz_marker_t*);

void overlayaz_marker_set_longitude(overlayaz_marker_t*, gdouble);
gdouble overlayaz_marker_get_longitude(const overlayaz_marker_t*);

void overlayaz_marker_set_tick(overlayaz_marker_t*, enum overlayaz_marker_tick);
enum overlayaz_marker_tick overlayaz_marker_get_tick(const overlayaz_marker_t*);

void overlayaz_marker_set_font(overlayaz_marker_t*, const gchar*);
const overlayaz_font_t* overlayaz_marker_get_font(const overlayaz_marker_t*);

void overlayaz_marker_set_font_color(overlayaz_marker_t*, const gchar*);
void overlayaz_marker_set_font_color_rgba(overlayaz_marker_t*, const GdkRGBA*);
const GdkRGBA* overlayaz_marker_get_font_color(const overlayaz_marker_t*);

void overlayaz_marker_set_position(overlayaz_marker_t*, gdouble);
gdouble overlayaz_marker_get_position(const overlayaz_marker_t*);

void overlayaz_marker_set_active(overlayaz_marker_t*, gboolean);
gboolean overlayaz_marker_get_active(const overlayaz_marker_t*);

void overlayaz_marker_set_show_azimuth(overlayaz_marker_t*, gboolean);
gboolean overlayaz_marker_get_show_azimuth(const overlayaz_marker_t*);

void overlayaz_marker_set_show_distance(overlayaz_marker_t*, gboolean);
gboolean overlayaz_marker_get_show_distance(const overlayaz_marker_t*);

#endif
