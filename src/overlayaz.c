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
#include "overlayaz.h"
#include "overlayaz-default.h"
#include "marker-list.h"
#include "font.h"
#include "geo.h"

#define OVERLAYAZ_INVALID_DATA  NAN
#define OVERLAYAZ_INVALID_RATIO 0.0

struct overlayaz_ref
{
    struct overlayaz_location location[OVERLAYAZ_REF_IDS];
    gdouble position[OVERLAYAZ_REF_IDS];
    gdouble angle[OVERLAYAZ_REF_IDS];
    gdouble ratio;
};

struct overlayaz
{
    gchar *filename;
    GdkPixbuf *pixbuf;
    gint width;
    gint height;
    gboolean changed;

    /* Profile */
    gdouble rotation;
    struct overlayaz_location location;
    struct overlayaz_ref ref[OVERLAYAZ_REF_TYPES];
    gboolean grid[OVERLAYAZ_REF_TYPES];
    gdouble grid_step[OVERLAYAZ_REF_TYPES];
    gdouble grid_position[OVERLAYAZ_REF_TYPES];
    gdouble grid_width;
    GdkRGBA grid_color;
    overlayaz_font_t *grid_font;
    GdkRGBA grid_font_color;
    GtkListStore *marker_list;
};

static void ref_update(overlayaz_t*, enum overlayaz_ref_type);
static void marker_changed(overlayaz_t*);
static inline gboolean overlayaz_is_valid(gdouble);


overlayaz_t*
overlayaz_new(void)
{
    overlayaz_t *o = g_malloc0(sizeof(overlayaz_t));

    o->grid_font = overlayaz_font_new(OVERLAYAZ_DEFAULT_GRID_FONT);
    o->marker_list = overlayaz_marker_list_new();
    g_signal_connect_swapped(o->marker_list, "row-changed", G_CALLBACK(marker_changed), o);
    g_signal_connect_swapped(o->marker_list, "row-inserted", G_CALLBACK(marker_changed), o);
    g_signal_connect_swapped(o->marker_list, "row-deleted", G_CALLBACK(marker_changed), o);
    g_signal_connect_swapped(o->marker_list, "rows-reordered", G_CALLBACK(marker_changed), o);

    overlayaz_reset(o);
    return o;
}

void
overlayaz_free(overlayaz_t *o)
{
    g_free(o->filename);
    if (o->pixbuf)
        g_object_unref(o->pixbuf);
    overlayaz_font_free(o->grid_font);
    overlayaz_marker_list_free(o->marker_list);
    g_free(o);
}

void
overlayaz_reset(overlayaz_t *o)
{
    overlayaz_set_filename(o, NULL);
    overlayaz_set_pixbuf(o, NULL);
    overlayaz_set_rotation(o, OVERLAYAZ_DEFAULT_ROTATION);
    overlayaz_set_location(o, &(struct overlayaz_location){OVERLAYAZ_DEFAULT_LATITUDE, OVERLAYAZ_DEFAULT_LONGITUDE, OVERLAYAZ_DEFAULT_ALTITUDE});
    overlayaz_set_ref_none(o, OVERLAYAZ_REF_AZ);
    overlayaz_set_ref_none(o, OVERLAYAZ_REF_EL);

    overlayaz_set_grid(o, OVERLAYAZ_REF_AZ, OVERLAYAZ_DEFAULT_GRID_AZIMUTH);
    overlayaz_set_grid(o, OVERLAYAZ_REF_EL, OVERLAYAZ_DEFAULT_GRID_ELEVATION);
    overlayaz_set_grid_step(o, OVERLAYAZ_REF_AZ, OVERLAYAZ_DEFAULT_GRID_AZIMUTH_STEP);
    overlayaz_set_grid_step(o, OVERLAYAZ_REF_EL, OVERLAYAZ_DEFAULT_GRID_ELEVATION_STEP);
    overlayaz_set_grid_position(o, OVERLAYAZ_REF_AZ, OVERLAYAZ_DEFAULT_GRID_POSITION_AZIMUTH);
    overlayaz_set_grid_position(o, OVERLAYAZ_REF_EL, OVERLAYAZ_DEFAULT_GRID_POSITION_ELEVATION);
    overlayaz_set_grid_width(o, OVERLAYAZ_DEFAULT_GRID_WIDTH);
    gdk_rgba_parse(&o->grid_color, OVERLAYAZ_DEFAULT_GRID_COLOR);
    overlayaz_set_grid_font(o, OVERLAYAZ_DEFAULT_GRID_FONT);
    gdk_rgba_parse(&o->grid_font_color, OVERLAYAZ_DEFAULT_GRID_FONT_COLOR);

    overlayaz_marker_list_clear(o->marker_list);

    o->changed = FALSE;
}

gboolean
overlayaz_changed(const overlayaz_t *o)
{
    return o->changed;
}

void
overlayaz_unchanged(overlayaz_t *o)
{
    o->changed = FALSE;
}

void
overlayaz_set_filename(overlayaz_t *o,
                       const gchar *filename)
{
    if (g_strcmp0(o->filename, filename))
    {
        g_free(o->filename);
        o->filename = g_strdup(filename);
        o->changed = TRUE;
    }
}

const gchar*
overlayaz_get_filename(const overlayaz_t *o)
{
    return o->filename;
}

void
overlayaz_set_pixbuf(overlayaz_t *o,
                     GdkPixbuf   *pixbuf)
{
    if (o->pixbuf)
        g_object_unref(o->pixbuf);

    o->pixbuf = pixbuf;
    o->width = pixbuf ? gdk_pixbuf_get_width(pixbuf) : 0;
    o->height = pixbuf ? gdk_pixbuf_get_height(pixbuf) : 0;
    o->changed = TRUE;
}

const GdkPixbuf*
overlayaz_get_pixbuf(const overlayaz_t *o)
{
    return o->pixbuf;
}

gint
overlayaz_get_width(const overlayaz_t *o)
{
    return o->width;
}

gint
overlayaz_get_height(const overlayaz_t *o)
{
    return o->height;
}

void
overlayaz_set_rotation(overlayaz_t *o,
                       gdouble      rotation)
{
    rotation = fmod(rotation, 360.0);
    if (rotation > 180.0)
        rotation -= 360.0;
    if (rotation < -180.0)
        rotation += 360.0;

    if (o->rotation != rotation)
    {
        o->rotation = rotation;
        o->changed = TRUE;
    }
}

gdouble
overlayaz_get_rotation(const overlayaz_t *o)
{
    return o->rotation;
}

void
overlayaz_set_location(overlayaz_t                     *o,
                       const struct overlayaz_location *l)
{
    if (o->location.latitude != l->latitude ||
        o->location.longitude != l->longitude ||
        o->location.altitude != l->altitude)
    {
        o->location = *l;

        if(o->location.latitude == 0.0 &&
           o->location.longitude == 0.0)
        {
            o->location.latitude = NAN;
            o->location.longitude = NAN;
        }

        ref_update(o, OVERLAYAZ_REF_AZ);
        ref_update(o, OVERLAYAZ_REF_EL);
        o->changed = TRUE;
    }
}

gboolean
overlayaz_get_location(const overlayaz_t         *o,
                       struct overlayaz_location *out)
{
    if (out)
        *out = o->location;

    return overlayaz_is_valid(o->location.latitude) &&
           overlayaz_is_valid(o->location.longitude);
}

void
overlayaz_set_ref_none(overlayaz_t             *o,
                       enum overlayaz_ref_type  type)
{
    gint i;

    for (i = 0; i < OVERLAYAZ_REF_IDS; i++)
    {
        o->ref[type].location[i].latitude = OVERLAYAZ_DEFAULT_LATITUDE;
        o->ref[type].location[i].longitude = OVERLAYAZ_DEFAULT_LONGITUDE;
        o->ref[type].location[i].altitude = OVERLAYAZ_DEFAULT_ALTITUDE;
        o->ref[type].position[i] = OVERLAYAZ_INVALID_DATA;
        o->ref[type].angle[i] = OVERLAYAZ_INVALID_DATA;
    }

    o->ref[type].ratio = OVERLAYAZ_INVALID_RATIO;
    o->changed = TRUE;
}

void
overlayaz_set_ref_one(overlayaz_t                     *o,
                      enum overlayaz_ref_type          type,
                      const struct overlayaz_location *location,
                      gdouble                          position,
                      gdouble                          ratio)
{
    o->ref[type].location[OVERLAYAZ_REF_A] = *location;
    o->ref[type].position[OVERLAYAZ_REF_A] = position;

    if(o->ref[type].location[OVERLAYAZ_REF_A].latitude == 0.0 &&
       o->ref[type].location[OVERLAYAZ_REF_A].longitude == 0.0)
    {
        o->ref[type].location[OVERLAYAZ_REF_A].latitude = NAN;
        o->ref[type].location[OVERLAYAZ_REF_A].longitude = NAN;
    }

    o->ref[type].location[OVERLAYAZ_REF_B].latitude = OVERLAYAZ_DEFAULT_LATITUDE;
    o->ref[type].location[OVERLAYAZ_REF_B].longitude = OVERLAYAZ_DEFAULT_LONGITUDE;
    o->ref[type].location[OVERLAYAZ_REF_B].altitude = OVERLAYAZ_DEFAULT_ALTITUDE;
    o->ref[type].position[OVERLAYAZ_REF_B] = OVERLAYAZ_INVALID_DATA;
    o->ref[type].ratio = ratio;
    ref_update(o, type);
    o->changed = TRUE;
}

void
overlayaz_set_ref_two(overlayaz_t                     *o,
                      enum overlayaz_ref_type          type,
                      const struct overlayaz_location *locationA,
                      gdouble                          positionA,
                      const struct overlayaz_location *locationB,
                      gdouble                          positionB)
{
    o->ref[type].location[OVERLAYAZ_REF_A] = *locationA;
    o->ref[type].position[OVERLAYAZ_REF_A] = positionA;

    if(o->ref[type].location[OVERLAYAZ_REF_A].latitude == 0.0 &&
       o->ref[type].location[OVERLAYAZ_REF_A].longitude == 0.0)
    {
        o->ref[type].location[OVERLAYAZ_REF_A].latitude = NAN;
        o->ref[type].location[OVERLAYAZ_REF_A].longitude = NAN;
    }

    o->ref[type].location[OVERLAYAZ_REF_B] = *locationB;
    o->ref[type].position[OVERLAYAZ_REF_B] = positionB;

    if(o->ref[type].location[OVERLAYAZ_REF_B].latitude == 0.0 &&
       o->ref[type].location[OVERLAYAZ_REF_B].longitude == 0.0)
    {
        o->ref[type].location[OVERLAYAZ_REF_B].latitude = NAN;
        o->ref[type].location[OVERLAYAZ_REF_B].longitude = NAN;
    }

    ref_update(o, type);
    o->changed = TRUE;
}

gboolean
overlayaz_get_ref_location(const overlayaz_t         *o,
                           enum overlayaz_ref_type    type,
                           enum overlayaz_ref_id      id,
                           struct overlayaz_location *out)
{
    if (out)
        *out = o->ref[type].location[id];

    return overlayaz_is_valid(o->ref[type].location[id].latitude) &&
           overlayaz_is_valid(o->ref[type].location[id].longitude);
}

gboolean
overlayaz_get_ref_position(const overlayaz_t       *o,
                           enum overlayaz_ref_type  type,
                           enum overlayaz_ref_id    id,
                           gdouble                 *out)
{
    if (out)
        *out = o->ref[type].position[id];

    return overlayaz_is_valid(o->ref[type].position[id]);
}

gboolean
overlayaz_get_ratio(const overlayaz_t       *o,
                    enum overlayaz_ref_type  type,
                    gdouble                 *out)
{
    if (out)
        *out = o->ref[type].ratio;

    return (o->ref[type].ratio != OVERLAYAZ_INVALID_RATIO);
}

gboolean
overlayaz_get_angle(const overlayaz_t       *o,
                    enum overlayaz_ref_type  type,
                    gdouble                  position,
                    gdouble                 *out)
{
    gdouble angle;
    gdouble diff;

    if (o->ref[type].ratio == OVERLAYAZ_INVALID_RATIO)
    {
        if (out)
            *out = OVERLAYAZ_INVALID_DATA;
        return FALSE;
    }

    if (out)
    {
        angle = o->ref[type].angle[OVERLAYAZ_REF_A];
        diff = (o->ref[type].position[OVERLAYAZ_REF_A] - position) / o->ref[type].ratio;

        if (type == OVERLAYAZ_REF_AZ)
            diff *= (-1);

        angle = fmod(angle + diff, 360.0);
        if (type == OVERLAYAZ_REF_AZ && angle < 0.0)
            angle += 360.0;

        *out = angle;
    }

    return TRUE;
}

gboolean
overlayaz_get_position(const overlayaz_t       *o,
                       enum overlayaz_ref_type  type,
                       gdouble                  angle,
                       gdouble                 *out)
{
    gdouble position;
    gdouble diff;

    if (o->ref[type].ratio == OVERLAYAZ_INVALID_RATIO)
    {
        if (out)
            *out = NAN;
        return FALSE;
    }

    angle = fmod(angle, 360.0);
    diff = (o->ref[type].angle[OVERLAYAZ_REF_A] - angle) * o->ref[type].ratio;
    position = o->ref[type].position[OVERLAYAZ_REF_A];
    position += (type == OVERLAYAZ_REF_AZ ? (-diff) : diff);

    /* Try overlapping */
    if (type == OVERLAYAZ_REF_AZ && position < 0)
    {
        diff = (o->ref[type].angle[OVERLAYAZ_REF_A] - angle - 360.0) * o->ref[type].ratio;
        position = o->ref[type].position[OVERLAYAZ_REF_A] - diff;

    }
    else if (type == OVERLAYAZ_REF_AZ && position > o->width)
    {
        diff = (o->ref[type].angle[OVERLAYAZ_REF_A] - angle + 360.0) * o->ref[type].ratio;
        position = o->ref[type].position[OVERLAYAZ_REF_A] - diff;
    }

    if (position < 0 || position > (type == OVERLAYAZ_REF_AZ ? o->width : o->height))
        position = OVERLAYAZ_INVALID_DATA;

    if (out)
        *out = position;

    return overlayaz_is_valid(position);
}

void
overlayaz_set_grid(overlayaz_t             *o,
                   enum overlayaz_ref_type  type,
                   gboolean                 value)
{
    if (o->grid[type] != value)
    {
        o->grid[type] = value;
        o->changed = TRUE;
    }
}

gboolean
overlayaz_get_grid(const overlayaz_t       *o,
                   enum overlayaz_ref_type  type)
{
    return o->grid[type];
}

void
overlayaz_set_grid_step(overlayaz_t             *o,
                        enum overlayaz_ref_type  type,
                        gdouble                  value)
{
    if (o->grid_step[type] != value)
    {
        o->grid_step[type] = value;
        o->changed = TRUE;
    }
}

gdouble
overlayaz_get_grid_step(const overlayaz_t       *o,
                        enum overlayaz_ref_type  type)
{
    return o->grid_step[type];
}

void
overlayaz_set_grid_position(overlayaz_t             *o,
                            enum overlayaz_ref_type  type,
                            gdouble                  value)
{
    if (o->grid_position[type] != value)
    {
        o->grid_position[type] = value;
        o->changed = TRUE;
    }
}

gdouble
overlayaz_get_grid_position(const overlayaz_t       *o,
                            enum overlayaz_ref_type  type)
{
    return o->grid_position[type];
}

void
overlayaz_set_grid_width(overlayaz_t *o,
                         gdouble      value)
{
    if (o->grid_width != value)
    {
        o->grid_width = value;
        o->changed = TRUE;
    }
}

gdouble
overlayaz_get_grid_width(const overlayaz_t *o)
{
    return o->grid_width;
}

void
overlayaz_set_grid_color(overlayaz_t   *o,
                         const GdkRGBA *color)
{
    if (!gdk_rgba_equal(&o->grid_color, color))
    {
        o->grid_color = *color;
        o->changed = TRUE;
    }
}

const GdkRGBA*
overlayaz_get_grid_color(const overlayaz_t *o)
{
    return &o->grid_color;
}

void
overlayaz_set_grid_font(overlayaz_t *o,
                        const gchar *font)
{
    if (g_strcmp0(overlayaz_font_get(o->grid_font), font) != 0)
    {
        overlayaz_font_set(o->grid_font, font);
        o->changed = TRUE;
    }
}

const overlayaz_font_t*
overlayaz_get_grid_font(const overlayaz_t *o)
{
    return o->grid_font;
}

void
overlayaz_set_grid_font_color(overlayaz_t   *o,
                              const GdkRGBA *color)
{
    if (!gdk_rgba_equal(&o->grid_font_color, color))
    {
        o->grid_font_color = *color;
        o->changed = TRUE;
    }
}

const GdkRGBA*
overlayaz_get_grid_font_color(const overlayaz_t *o)
{
    return &o->grid_font_color;
}

GtkListStore*
overlayaz_get_marker_list(const overlayaz_t *o)
{
    return o->marker_list;
}

static void
ref_update(overlayaz_t             *o,
           enum overlayaz_ref_type  type)
{
    gdouble ratio = OVERLAYAZ_INVALID_RATIO;
    gdouble angle[OVERLAYAZ_REF_IDS] = {OVERLAYAZ_INVALID_DATA, OVERLAYAZ_INVALID_DATA};
    gdouble offset;
    gint idx[2];
    gint i;

    if (overlayaz_is_valid(o->location.latitude) &&
        overlayaz_is_valid(o->location.longitude))
    {
        for (i = 0; i < OVERLAYAZ_REF_IDS; i++)
        {
            if (overlayaz_is_valid(o->ref[type].location[i].latitude) &&
                overlayaz_is_valid(o->ref[type].location[i].longitude))
            {
                if (type == OVERLAYAZ_REF_AZ)
                {
                    overlayaz_geo_inverse(o->location.latitude, o->location.longitude,
                                          o->ref[type].location[i].latitude, o->ref[type].location[i].longitude,
                                          &angle[i], NULL, NULL);
                }
                else if (type == OVERLAYAZ_REF_EL)
                {
                    overlayaz_geo_elevation(o->location.latitude, o->location.longitude, o->location.altitude,
                                            o->ref[type].location[i].latitude, o->ref[type].location[i].longitude, o->ref[type].location[i].altitude,
                                            &angle[i]);
                }
            }
        }
    }

    if (overlayaz_is_valid(o->ref[type].position[OVERLAYAZ_REF_A]) &&
        overlayaz_is_valid(angle[OVERLAYAZ_REF_A]))
    {
        if (overlayaz_is_valid(o->ref[type].position[OVERLAYAZ_REF_B]) &&
            overlayaz_is_valid(angle[OVERLAYAZ_REF_B]))
        {
            /* Keep incremental order of reference positions (image width or height) */
            idx[0] = (gint)(o->ref[type].position[OVERLAYAZ_REF_A] < o->ref[type].position[OVERLAYAZ_REF_B]);
            idx[1] = (gint)(o->ref[type].position[OVERLAYAZ_REF_A] >= o->ref[type].position[OVERLAYAZ_REF_B]);

            /* Allow overlapping for azimuth reference */
            if (type == OVERLAYAZ_REF_AZ && (angle[idx[0]] < angle[idx[1]]))
                offset = 360.0;
            else
                offset = 0.0;

            ratio = (o->ref[type].position[idx[0]] - o->ref[type].position[idx[1]]);
            ratio /= (offset + angle[idx[0]] - angle[idx[1]]);

            /* The calculated elevation ratio is negative,
             * but we want to keep it as a positive number. */
            if (type == OVERLAYAZ_REF_EL)
                ratio *= (-1);
        }
        else
        {
            ratio = o->ref[type].ratio;
        }
    }

    if (type == OVERLAYAZ_REF_AZ)
    {
        /* Boundary check:
         * Discard invalid ratio values (exceeding the image size) */
        if (ratio * 360.0 < overlayaz_get_width(o))
            ratio = OVERLAYAZ_INVALID_RATIO;
    }

    if (ratio <= 0.0)
        ratio = OVERLAYAZ_INVALID_RATIO;

    if (!overlayaz_is_valid(ratio))
        ratio = OVERLAYAZ_INVALID_RATIO;

    o->ref[type].angle[OVERLAYAZ_REF_A] = angle[OVERLAYAZ_REF_A];
    o->ref[type].angle[OVERLAYAZ_REF_B] = angle[OVERLAYAZ_REF_B];
    o->ref[type].ratio = ratio;
}

static void
marker_changed(overlayaz_t *o)
{
    o->changed = TRUE;
}

static inline gboolean
overlayaz_is_valid(gdouble value)
{
    return !isnan(value);
}
