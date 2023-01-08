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
#include "overlayaz.h"

#define GRID_COUNT_LIMIT 1000


gboolean
overlayaz_util_grid_calc(const overlayaz_t       *o,
                         enum overlayaz_ref_type  type,
                         gdouble                 *first,
                         gdouble                 *step,
                         gint                    *count)
{
    gdouble angle;
    gdouble ratio;
    gdouble boundary;
    gdouble position;

    if (!overlayaz_get_grid(o, type))
        return FALSE;

    if (!overlayaz_get_angle(o, type, 0.0, &angle))
        return FALSE;

    if (!overlayaz_get_ratio(o, type, &ratio))
        return FALSE;

    boundary = (type == OVERLAYAZ_REF_AZ) ? overlayaz_get_width(o) : overlayaz_get_height(o);
    angle = floor(angle / overlayaz_get_grid_step(o, type)) * overlayaz_get_grid_step(o, type);
    if (type == OVERLAYAZ_REF_AZ)
        angle += overlayaz_get_grid_step(o, type);

    if (!overlayaz_get_position(o, type, angle, &position))
        return FALSE;

    if (first)
        *first = angle;

    if (step)
        *step = overlayaz_get_grid_step(o, type);

    if (count)
    {
        *count = (gint)((boundary - position) / (ratio * overlayaz_get_grid_step(o, type))) + 1;
        *count = MIN(*count, GRID_COUNT_LIMIT);
    }

    return TRUE;
}

gdouble
overlayaz_util_fov_calc(gdouble  sensor_length,
                        gdouble  focal_length)
{
    gdouble fov = 2 * atan(sensor_length / (2.0 * focal_length)) * 180.0 / G_PI;
    return fov;
}
