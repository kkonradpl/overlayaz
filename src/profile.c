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
#include <json-c/json.h>
#include <glib/gstdio.h>
#include <math.h>
#include "overlayaz.h"
#include "profile.h"
#include "marker-iter.h"
#include "marker-list.h"

#define PROFILE_VERSION 1

#define PROFILE_READ_BUFFER 1024

#define PROFILE_KEY_VERSION                 "overlayaz"
#define PROFILE_KEY_ROTATION                "rotation"

#define PROFILE_KEY_LATITUDE                "latitude"
#define PROFILE_KEY_LONGITUDE               "longitude"
#define PROFILE_KEY_ALTITUDE                "altitude"

#define PROFILE_KEY_REF_AZIMUTH             "ref-azimuth"
#define PROFILE_KEY_REF_ELEVATION           "ref-elevation"
#define PROFILE_KEY_REF_FIRST_LATITUDE      "first-latitude"
#define PROFILE_KEY_REF_FIRST_LONGITUDE     "first-longitude"
#define PROFILE_KEY_REF_FIRST_ALTITUDE      "first-altitude"
#define PROFILE_KEY_REF_FIRST_POSITION      "first-position"
#define PROFILE_KEY_REF_SECOND_LATITUDE     "second-latitude"
#define PROFILE_KEY_REF_SECOND_LONGITUDE    "second-longitude"
#define PROFILE_KEY_REF_SECOND_ALTITUDE     "second-altitude"
#define PROFILE_KEY_REF_SECOND_POSITION     "second-position"
#define PROFILE_KEY_REF_RATIO               "ratio"

#define PROFILE_KEY_GRID                    "grid"
#define PROFILE_KEY_GRID_AZIMUTH            "azimuth"
#define PROFILE_KEY_GRID_ELEVATION          "elevation"
#define PROFILE_KEY_GRID_STEP_AZIMUTH       "step-azimuth"
#define PROFILE_KEY_GRID_STEP_ELEVATION     "step-elevation"
#define PROFILE_KEY_GRID_POSITION_AZIMUTH   "position-azimuth"
#define PROFILE_KEY_GRID_POSITION_ELEVATION "position-elevation"
#define PROFILE_KEY_GRID_WIDTH              "width"
#define PROFILE_KEY_GRID_COLOR              "color"
#define PROFILE_KEY_GRID_FONT               "font"
#define PROFILE_KEY_GRID_FONT_COLOR         "font-color"

#define PROFILE_KEY_MARKER                  "marker"
#define PROFILE_KEY_MARKER_NAME             "name"
#define PROFILE_KEY_MARKER_LATITUDE         "latitude"
#define PROFILE_KEY_MARKER_LONGITUDE        "longitude"
#define PROFILE_KEY_MARKER_TICK             "tick"
#define PROFILE_KEY_MARKER_FONT             "font"
#define PROFILE_KEY_MARKER_FONT_COLOR       "font-color"
#define PROFILE_KEY_MARKER_POSITION         "position"
#define PROFILE_KEY_MARKER_ACTIVE           "active"
#define PROFILE_KEY_MARKER_SHOW_AZIMUTH     "show-azimuth"
#define PROFILE_KEY_MARKER_SHOW_DISTANCE    "show-distance"

#define PROFILE_VALUE_MARKER_TICK_NONE      "none"
#define PROFILE_VALUE_MARKER_TICK_TOP       "top"
#define PROFILE_VALUE_MARKER_TICK_BOTTOM    "bottom"

static enum overlayaz_profile_load_error profile_parse(overlayaz_t*, json_object*);
static void profile_parse_ref(overlayaz_t*, enum overlayaz_ref_type, json_object*);
static void profile_parse_grid(overlayaz_t*, json_object*);
static void profile_parse_marker(overlayaz_t*, json_object*);

static json_object* profile_build(const overlayaz_t*);
static void profile_build_ref(const overlayaz_t*, enum overlayaz_ref_type, json_object*, const gchar*);
static void profile_build_grid(const overlayaz_t*, json_object*, const gchar*);
static void profile_build_markers(const overlayaz_t*, json_object*, const gchar*);

static gboolean profile_json_get_double(json_object*, gdouble*);
static json_object* profile_json_new_double(gdouble);


enum overlayaz_profile_load_error
overlayaz_profile_load(overlayaz_t *o,
                       const gchar *filename)
{
    FILE *fp;
    gchar buffer[PROFILE_READ_BUFFER];
    size_t n;
    json_tokener *json;
    json_object *root;
    enum json_tokener_error err;
    gboolean ret;

    fp = g_fopen(filename, "r");

    if (fp == NULL)
        return OVERLAYAZ_PROFILE_LOAD_ERROR_OPEN;

    json = json_tokener_new();
    do
    {
        n = fread(buffer, sizeof(gchar), PROFILE_READ_BUFFER, fp);
        root = json_tokener_parse_ex(json, buffer, n);

        if (feof(fp))
        {
            err = json_tokener_get_error(json);
            break;
        }
    } while ((err = json_tokener_get_error(json)) == json_tokener_continue);

    if (err != json_tokener_success)
    {
        if (root)
            json_object_put(root);
        json_tokener_free(json);
        return OVERLAYAZ_PROFILE_LOAD_ERROR_PARSE;
    }
    fclose(fp);

    ret = profile_parse(o, root);
    json_object_put(root);
    json_tokener_free(json);
    return ret;

}

enum overlayaz_profile_save_error
overlayaz_profile_save(const overlayaz_t *o,
                       const gchar       *filename)
{
    FILE *fp;
    json_object *json;
    const gchar *json_string;
    size_t json_length;
    size_t wrote;

    fp = fopen(filename, "w");
    if (fp == NULL)
        return OVERLAYAZ_PROFILE_SAVE_ERROR_OPEN;

    json = profile_build(o);
    json_string = json_object_to_json_string_ext(json, JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED);
    json_length = strlen(json_string);
    wrote = fwrite(json_string, sizeof(gchar), json_length, fp);
    fclose(fp);

    json_object_put(json);
    return (json_length == wrote) ? OVERLAYAZ_PROFILE_SAVE_OK : OVERLAYAZ_PROFILE_SAVE_ERROR_WRITE;
}

static enum overlayaz_profile_load_error
profile_parse(overlayaz_t *o,
              json_object *root)
{
    json_object *object, *arr_object;
    gdouble value;
    size_t i;
    struct overlayaz_location location = {NAN, NAN,0.0};

    if (!json_object_object_get_ex(root, PROFILE_KEY_VERSION, &object))
        return OVERLAYAZ_PROFILE_LOAD_ERROR_FORMAT;

    if (json_object_get_int(object) != PROFILE_VERSION)
        return OVERLAYAZ_PROFILE_LOAD_ERROR_VERSION;

    if (json_object_object_get_ex(root, PROFILE_KEY_ROTATION, &object) &&
       profile_json_get_double(object, &value))
    {
        overlayaz_set_rotation(o, value);
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_LATITUDE, &object))
        profile_json_get_double(object, &location.latitude);

    if (json_object_object_get_ex(root, PROFILE_KEY_LONGITUDE, &object))
        profile_json_get_double(object, &location.longitude);

    if (json_object_object_get_ex(root, PROFILE_KEY_ALTITUDE, &object))
        profile_json_get_double(object, &location.altitude);

    overlayaz_set_location(o, &location);

    if (json_object_object_get_ex(root, PROFILE_KEY_REF_AZIMUTH, &object) &&
        json_object_is_type(object, json_type_object))
    {
        profile_parse_ref(o, OVERLAYAZ_REF_AZ, object);
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_REF_ELEVATION, &object) &&
        json_object_is_type(object, json_type_object))
    {
        profile_parse_ref(o, OVERLAYAZ_REF_EL, object);
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_GRID, &object) &&
        json_object_is_type(object, json_type_object))
    {
        profile_parse_grid(o, object);
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_MARKER, &object) &&
        json_object_is_type(object, json_type_array))
    {
        for (i = 0; i < json_object_array_length(object); i++)
        {
            arr_object = json_object_array_get_idx(object, i);
            if (json_object_is_type(arr_object, json_type_object))
                profile_parse_marker(o, arr_object);
        }
    }

    return OVERLAYAZ_PROFILE_LOAD_OK;
}

static void
profile_parse_ref(overlayaz_t             *o,
                  enum overlayaz_ref_type  type,
                  json_object             *root)
{
    json_object *obj_position, *obj_latitude, *obj_longitude, *obj_altitude;
    json_object *obj_ratio;
    struct overlayaz_location locationA = { NAN, NAN, 0.0 };
    struct overlayaz_location locationB = { NAN, NAN, 0.0 };
    gdouble positionA = NAN;
    gdouble positionB = NAN;
    gdouble ratio;

    if (json_object_object_get_ex(root, PROFILE_KEY_REF_FIRST_POSITION, &obj_position))
        profile_json_get_double(obj_position, &positionA);

    if (json_object_object_get_ex(root, PROFILE_KEY_REF_FIRST_LATITUDE, &obj_latitude))
        profile_json_get_double(obj_latitude, &locationA.latitude);

    if (json_object_object_get_ex(root, PROFILE_KEY_REF_FIRST_LONGITUDE, &obj_longitude))
        profile_json_get_double(obj_longitude, &locationA.longitude);

    if (json_object_object_get_ex(root, PROFILE_KEY_REF_FIRST_ALTITUDE, &obj_altitude))
        profile_json_get_double(obj_altitude, &locationA.altitude);

    if (json_object_object_get_ex(root, PROFILE_KEY_REF_SECOND_POSITION, &obj_position))
        profile_json_get_double(obj_position, &positionB);

    if (json_object_object_get_ex(root, PROFILE_KEY_REF_SECOND_LATITUDE, &obj_position))
        profile_json_get_double(obj_position, &locationB.latitude);

    if (json_object_object_get_ex(root, PROFILE_KEY_REF_SECOND_LONGITUDE, &obj_longitude))
        profile_json_get_double(obj_longitude, &locationB.longitude);

    if (json_object_object_get_ex(root, PROFILE_KEY_REF_SECOND_ALTITUDE, &obj_altitude))
        profile_json_get_double(obj_altitude, &locationB.altitude);

    if (json_object_object_get_ex(root, PROFILE_KEY_REF_RATIO, &obj_ratio))
    {
        profile_json_get_double(obj_ratio, &ratio);
        overlayaz_set_ref_one(o, type,
                              &locationA, positionA,
                              ratio);
    }
    else
    {
        overlayaz_set_ref_two(o, type,
                              &locationA, positionA,
                              &locationB, positionB);
    }
}

static void
profile_parse_grid(overlayaz_t *o,
                   json_object *root)
{
    json_object *object;
    GdkRGBA color;
    gdouble value;

    if (json_object_object_get_ex(root, PROFILE_KEY_GRID_AZIMUTH, &object) &&
        json_object_is_type(object, json_type_boolean))
    {
        overlayaz_set_grid(o, OVERLAYAZ_REF_AZ, json_object_get_boolean(object));
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_GRID_ELEVATION, &object) &&
        json_object_is_type(object, json_type_boolean))
    {
        overlayaz_set_grid(o, OVERLAYAZ_REF_EL, json_object_get_boolean(object));
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_GRID_STEP_AZIMUTH, &object) &&
        profile_json_get_double(object, &value))
    {
        overlayaz_set_grid_step(o, OVERLAYAZ_REF_AZ, value);
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_GRID_STEP_ELEVATION, &object) &&
        profile_json_get_double(object, &value))
    {
        overlayaz_set_grid_step(o, OVERLAYAZ_REF_EL, value);
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_GRID_POSITION_AZIMUTH, &object) &&
        profile_json_get_double(object, &value))
    {
        overlayaz_set_grid_position(o, OVERLAYAZ_REF_AZ, value);
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_GRID_POSITION_ELEVATION, &object) &&
        profile_json_get_double(object, &value))
    {
        overlayaz_set_grid_position(o, OVERLAYAZ_REF_EL, value);
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_GRID_COLOR, &object) &&
        json_object_is_type(object, json_type_string) &&
        gdk_rgba_parse(&color, json_object_get_string(object)))
    {
        overlayaz_set_grid_color(o, &color);
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_GRID_WIDTH, &object) &&
        profile_json_get_double(object, &value))
    {
        overlayaz_set_grid_width(o, value);
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_GRID_FONT, &object) &&
        json_object_is_type(object, json_type_string))
    {
        overlayaz_set_grid_font(o, json_object_get_string(object));
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_GRID_FONT_COLOR, &object) &&
        json_object_is_type(object, json_type_string) &&
        gdk_rgba_parse(&color, json_object_get_string(object)))
    {
        overlayaz_set_grid_font_color(o, &color);
    }
}

static void
profile_parse_marker(overlayaz_t *o,
                     json_object *root)
{
    json_object *object;
    gdouble value;
    overlayaz_marker_t *m = overlayaz_marker_new();

    if (json_object_object_get_ex(root, PROFILE_KEY_MARKER_NAME, &object) &&
        json_object_is_type(object, json_type_string))
    {
        overlayaz_marker_set_name(m, json_object_get_string(object));
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_MARKER_LATITUDE, &object) &&
        profile_json_get_double(object, &value))
    {
        overlayaz_marker_set_latitude(m, value);
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_MARKER_LONGITUDE, &object) &&
        profile_json_get_double(object, &value))
    {
        overlayaz_marker_set_longitude(m, value);
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_MARKER_TICK, &object) &&
        json_object_is_type(object, json_type_string))
    {
        if (g_strcmp0(json_object_get_string(object), PROFILE_VALUE_MARKER_TICK_TOP) == 0)
            overlayaz_marker_set_tick(m, OVERLAYAZ_MARKER_TICK_TOP);
        else if (g_strcmp0(json_object_get_string(object), PROFILE_VALUE_MARKER_TICK_BOTTOM) == 0)
            overlayaz_marker_set_tick(m, OVERLAYAZ_MARKER_TICK_BOTTOM);
        else
            overlayaz_marker_set_tick(m, OVERLAYAZ_MARKER_TICK_NONE);
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_MARKER_FONT, &object) &&
        json_object_is_type(object, json_type_string))
    {
        overlayaz_marker_set_font(m, json_object_get_string(object));
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_MARKER_FONT_COLOR, &object) &&
        json_object_is_type(object, json_type_string))
    {
        overlayaz_marker_set_font_color(m, json_object_get_string(object));
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_MARKER_POSITION, &object) &&
        profile_json_get_double(object, &value))
    {
        overlayaz_marker_set_position(m, value);
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_MARKER_ACTIVE, &object) &&
        json_object_is_type(object, json_type_boolean))
    {
        overlayaz_marker_set_active(m, json_object_get_boolean(object));
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_MARKER_SHOW_AZIMUTH, &object) &&
        json_object_is_type(object, json_type_boolean))
    {
        overlayaz_marker_set_show_azimuth(m, json_object_get_boolean(object));
    }

    if (json_object_object_get_ex(root, PROFILE_KEY_MARKER_SHOW_DISTANCE, &object) &&
        json_object_is_type(object, json_type_boolean))
    {
        overlayaz_marker_set_show_distance(m, json_object_get_boolean(object));
    }

    overlayaz_marker_list_add(overlayaz_get_marker_list(o), m);
}

static json_object*
profile_build(const overlayaz_t *o)
{
    json_object *root;
    struct overlayaz_location location;
    gdouble val;

    root = json_object_new_object();
    json_object_object_add(root, PROFILE_KEY_VERSION, json_object_new_int(PROFILE_VERSION));

    val = overlayaz_get_rotation(o);
    json_object_object_add(root, PROFILE_KEY_ROTATION, profile_json_new_double(val));

    if (overlayaz_get_location(o, &location))
    {
        json_object_object_add(root, PROFILE_KEY_LATITUDE, profile_json_new_double(location.latitude));
        json_object_object_add(root, PROFILE_KEY_LONGITUDE, profile_json_new_double(location.longitude));
        json_object_object_add(root, PROFILE_KEY_ALTITUDE, profile_json_new_double(location.altitude));
    }

    profile_build_ref(o, OVERLAYAZ_REF_AZ, root, PROFILE_KEY_REF_AZIMUTH);
    profile_build_ref(o, OVERLAYAZ_REF_EL, root, PROFILE_KEY_REF_ELEVATION);
    profile_build_grid(o, root, PROFILE_KEY_GRID);
    profile_build_markers(o, root, PROFILE_KEY_MARKER);

    return root;
}

static void
profile_build_ref(const overlayaz_t       *o,
                  enum overlayaz_ref_type  type,
                  json_object             *root,
                  const gchar             *key)
{
    json_object *root_ref;
    struct overlayaz_location location;
    gboolean ref_loc, ref_pos;
    gdouble position;
    gdouble ratio;

    ref_loc = overlayaz_get_ref_location(o, type, OVERLAYAZ_REF_A, &location);
    ref_pos = overlayaz_get_ref_position(o, type, OVERLAYAZ_REF_A, &position);

    if (!ref_loc && !ref_pos)
        return;

    root_ref = json_object_new_object();

    if (ref_pos)
        json_object_object_add(root_ref, PROFILE_KEY_REF_FIRST_POSITION, profile_json_new_double(position));
    if (ref_loc)
    {
        json_object_object_add(root_ref, PROFILE_KEY_REF_FIRST_LATITUDE, profile_json_new_double(location.latitude));
        json_object_object_add(root_ref, PROFILE_KEY_REF_FIRST_LONGITUDE, profile_json_new_double(location.longitude));
    }
    if (type == OVERLAYAZ_REF_EL)
        json_object_object_add(root_ref, PROFILE_KEY_REF_FIRST_ALTITUDE, profile_json_new_double(location.altitude));

    ref_loc = overlayaz_get_ref_location(o, type, OVERLAYAZ_REF_B, &location);
    ref_pos = overlayaz_get_ref_position(o, type, OVERLAYAZ_REF_B, &position);

    if (!ref_loc && !ref_pos)
    {
        if (overlayaz_get_ratio(o, type, &ratio))
            json_object_object_add(root_ref, PROFILE_KEY_REF_RATIO, profile_json_new_double(ratio));
    }
    else
    {
        if (ref_pos)
            json_object_object_add(root_ref, PROFILE_KEY_REF_SECOND_POSITION, profile_json_new_double(position));
        if (ref_loc)
        {
            json_object_object_add(root_ref, PROFILE_KEY_REF_SECOND_LATITUDE, profile_json_new_double(location.latitude));
            json_object_object_add(root_ref, PROFILE_KEY_REF_SECOND_LONGITUDE, profile_json_new_double(location.longitude));
        }
        if (type == OVERLAYAZ_REF_EL)
            json_object_object_add(root_ref, PROFILE_KEY_REF_SECOND_ALTITUDE, profile_json_new_double(location.altitude));
    }

    json_object_object_add(root, key, root_ref);
}

static void
profile_build_grid(const overlayaz_t *o,
                   json_object       *root,
                   const gchar       *key)
{
    json_object *root_grid;
    gchar *color;

    root_grid = json_object_new_object();

    json_object_object_add(root_grid, PROFILE_KEY_GRID_AZIMUTH, json_object_new_boolean(overlayaz_get_grid(o, OVERLAYAZ_REF_AZ)));
    json_object_object_add(root_grid, PROFILE_KEY_GRID_ELEVATION, json_object_new_boolean(overlayaz_get_grid(o, OVERLAYAZ_REF_EL)));
    json_object_object_add(root_grid, PROFILE_KEY_GRID_STEP_AZIMUTH, profile_json_new_double(overlayaz_get_grid_step(o, OVERLAYAZ_REF_AZ)));
    json_object_object_add(root_grid, PROFILE_KEY_GRID_STEP_ELEVATION, profile_json_new_double(overlayaz_get_grid_step(o, OVERLAYAZ_REF_EL)));
    json_object_object_add(root_grid, PROFILE_KEY_GRID_POSITION_AZIMUTH, profile_json_new_double(overlayaz_get_grid_position(o, OVERLAYAZ_REF_AZ)));
    json_object_object_add(root_grid, PROFILE_KEY_GRID_POSITION_ELEVATION, profile_json_new_double(overlayaz_get_grid_position(o, OVERLAYAZ_REF_EL)));
    json_object_object_add(root_grid, PROFILE_KEY_GRID_WIDTH, profile_json_new_double(overlayaz_get_grid_width(o)));

    color = gdk_rgba_to_string(overlayaz_get_grid_color(o));
    json_object_object_add(root_grid, PROFILE_KEY_GRID_COLOR, json_object_new_string(color));
    g_free(color);

    json_object_object_add(root_grid, PROFILE_KEY_GRID_FONT, json_object_new_string(overlayaz_font_get(overlayaz_get_grid_font(o))));

    color = gdk_rgba_to_string(overlayaz_get_grid_font_color(o));
    json_object_object_add(root_grid, PROFILE_KEY_GRID_FONT_COLOR, json_object_new_string(color));
    g_free(color);

    json_object_object_add(root, key, root_grid);
}

static void
profile_build_markers(const overlayaz_t    *o,
                      json_object          *root,
                      const gchar          *key)
{
    json_object *array, *root_marker;
    overlayaz_marker_iter_t *iter;
    const overlayaz_marker_t *m;
    gchar *color;

    iter = overlayaz_marker_iter_new(overlayaz_get_marker_list(o), &m);
    if (iter == NULL)
        return;

    array = json_object_new_array();
    do
    {
        root_marker = json_object_new_object();
        color = gdk_rgba_to_string(overlayaz_marker_get_font_color(m));
        json_object_object_add(root_marker, PROFILE_KEY_MARKER_NAME, json_object_new_string(overlayaz_marker_get_name(m)));
        json_object_object_add(root_marker, PROFILE_KEY_MARKER_LATITUDE, profile_json_new_double(overlayaz_marker_get_latitude(m)));
        json_object_object_add(root_marker, PROFILE_KEY_MARKER_LONGITUDE, profile_json_new_double(overlayaz_marker_get_longitude(m)));
        switch (overlayaz_marker_get_tick(m))
        {
        case OVERLAYAZ_MARKER_TICK_TOP:
            json_object_object_add(root_marker, PROFILE_KEY_MARKER_TICK, json_object_new_string(PROFILE_VALUE_MARKER_TICK_TOP));
            break;
        case OVERLAYAZ_MARKER_TICK_BOTTOM:
            json_object_object_add(root_marker, PROFILE_KEY_MARKER_TICK, json_object_new_string(PROFILE_VALUE_MARKER_TICK_BOTTOM));
            break;
        default:
            json_object_object_add(root_marker, PROFILE_KEY_MARKER_TICK, json_object_new_string(PROFILE_VALUE_MARKER_TICK_NONE));
            break;
        }
        json_object_object_add(root_marker, PROFILE_KEY_MARKER_FONT, json_object_new_string(overlayaz_font_get(overlayaz_marker_get_font(m))));
        json_object_object_add(root_marker, PROFILE_KEY_MARKER_FONT_COLOR, json_object_new_string(color));
        json_object_object_add(root_marker, PROFILE_KEY_MARKER_POSITION, profile_json_new_double(overlayaz_marker_get_position(m)));
        json_object_object_add(root_marker, PROFILE_KEY_MARKER_ACTIVE, json_object_new_boolean(overlayaz_marker_get_active(m)));
        json_object_object_add(root_marker, PROFILE_KEY_MARKER_SHOW_AZIMUTH, json_object_new_boolean(overlayaz_marker_get_show_azimuth(m)));
        json_object_object_add(root_marker, PROFILE_KEY_MARKER_SHOW_DISTANCE, json_object_new_boolean(overlayaz_marker_get_show_distance(m)));
        json_object_array_add(array, root_marker);
        g_free(color);
    } while (overlayaz_marker_iter_next(iter, &m));

    json_object_object_add(root, key, array);
    overlayaz_marker_iter_free(iter);
}

static gboolean
profile_json_get_double(json_object *obj,
                        gdouble     *value)
{
    if (json_object_is_type(obj, json_type_double))
    {
        *value = json_object_get_double(obj);
        return TRUE;
    }
    else if (json_object_is_type(obj, json_type_int))
    {
        *value = (gdouble)json_object_get_int(obj);
        return TRUE;
    }

    return FALSE;
}

static json_object*
profile_json_new_double(gdouble value)
{
    gchar output[G_ASCII_DTOSTR_BUF_SIZE];
    g_ascii_formatd(output, sizeof(output), "%.10g", value);
    return json_object_new_double_s(value, output);
}
