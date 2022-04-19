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

#include <osmgpsmap-1.0/osm-gps-map.h>
#include <gtk/gtk.h>
#include <math.h>
#include "ui.h"
#include "ui-view-map.h"
#include "icon.h"
#include "geo.h"
#include "conf.h"
#include "marker-iter.h"
#include "grid-helper.h"

#define UI_VIEW_MAP_ICON_SIZE 41
#define UI_VIEW_MAP_PATH_STEP 2000
#define UI_VIEW_MAP_ZOOM_MIN 3
#define UI_VIEW_MAP_ZOOM_MAX 19

#define UI_VIEW_MAP_MARKER_CACHE 100

struct overlayaz_ui_view_map
{
    overlayaz_ui_t *ui;
    OsmGpsMap *map;
    const overlayaz_t *o;
    gboolean map_busy;

    /* Pixbuf caches */
    GdkPixbuf *pixbuf_home;
    GdkPixbuf *pixbuf_ref[OVERLAYAZ_REF_TYPES][OVERLAYAZ_REF_IDS];
    GdkPixbuf *pixbuf_marker[UI_VIEW_MAP_MARKER_CACHE];

    /* Map images */
    OsmGpsMapImage *img_home;
    OsmGpsMapImage *img_ref[OVERLAYAZ_REF_TYPES][OVERLAYAZ_REF_IDS];
    GSList *img_marker;

    /* Map tracks */
    OsmGpsMapTrack *track_bound[2];
    GSList *track_grid;
    GSList *track_marker;
};

static GdkRGBA color_grid = { .red = 1.0, .blue = 1.0, .green = 1.0, .alpha = 0.2 };
static GdkRGBA color_bound = { .red = 0.0, .blue = 0.0, .green = 0.0, .alpha = 0.2 };
static GdkRGBA color_marker = { .red = 1.0, .blue = 0.0, .green = 0.0, .alpha = 0.2 };

static void ui_view_map_update_grid(overlayaz_ui_view_map_t*);
static void ui_view_map_update_markers(overlayaz_ui_view_map_t*);
static void ui_view_map_remove_image(OsmGpsMap*, OsmGpsMapImage**);
static void ui_view_map_remove_track(OsmGpsMap*, OsmGpsMapTrack**);
static OsmGpsMapTrack* ui_view_map_track_new(gdouble, gdouble, gdouble, gdouble, GdkRGBA*);

static gboolean ui_view_map_press(GtkWidget*, GdkEventButton*, overlayaz_ui_view_map_t*);
static gboolean ui_view_map_scroll(GtkWidget*, GdkEventScroll*, overlayaz_ui_view_map_t*);
static gboolean ui_view_map_release(GtkWidget*, GdkEventButton*, overlayaz_ui_view_map_t*);
static gboolean ui_view_map_motion(GtkWidget*, GdkEventMotion*, overlayaz_ui_view_map_t*);
static gboolean ui_view_map_leave(GtkWidget*, GdkEvent*, overlayaz_ui_view_map_t*);
static void ui_view_map_measure(overlayaz_ui_view_map_t*, gdouble, gdouble);


overlayaz_ui_view_map_t*
overlayaz_ui_view_map_new(overlayaz_ui_t    *ui,
                          GtkWidget         *map,
                          const overlayaz_t *o)
{
    overlayaz_ui_view_map_t *ui_map = g_malloc0(sizeof(overlayaz_ui_view_map_t));
    GValue cache = G_VALUE_INIT;
    GValue source = G_VALUE_INIT;
    gint t, i;

    ui_map->ui = ui;
    ui_map->map = OSM_GPS_MAP(map);
    ui_map->o = o;

    g_value_init(&cache, G_TYPE_STRING);
    g_value_set_string(&cache, OSM_GPS_MAP_CACHE_AUTO);
    g_object_set_property(G_OBJECT(ui_map->map), "tile-cache", &cache);
    g_value_unset(&cache);

    g_value_init(&source, G_TYPE_INT);
    g_value_set_int(&source, OSM_GPS_MAP_SOURCE_GOOGLE_HYBRID);
    g_object_set_property(G_OBJECT(ui_map->map), "map-source", &source);
    g_value_unset(&source);

    /* Pixbuf caches */
    ui_map->pixbuf_home = overlayaz_icon_home(UI_VIEW_MAP_ICON_SIZE);
    for (t = 0; t < OVERLAYAZ_REF_TYPES; t++)
        for (i = 0; i < OVERLAYAZ_REF_IDS; i++)
            ui_map->pixbuf_ref[t][i] = overlayaz_icon_ref(UI_VIEW_MAP_ICON_SIZE, t, i);
    /* Pixbufs for markers will be created on-demand */

    g_signal_connect(ui_map->map, "button-press-event", G_CALLBACK(ui_view_map_press), ui_map);
    g_signal_connect(ui_map->map, "scroll-event", G_CALLBACK(ui_view_map_scroll), ui_map);
    g_signal_connect(ui_map->map, "button-release-event", G_CALLBACK(ui_view_map_release), ui_map);
    g_signal_connect(ui_map->map, "motion-notify-event", G_CALLBACK(ui_view_map_motion), ui_map);
    g_signal_connect(ui_map->map, "leave-notify-event", G_CALLBACK(ui_view_map_leave), ui_map);

    return ui_map;
}

void
overlayaz_ui_view_map_free(overlayaz_ui_view_map_t *ui_map)
{
    gint t, i;

    g_object_unref(ui_map->pixbuf_home);
    for (t = 0; t < OVERLAYAZ_REF_TYPES; t++)
        for (i = 0; i < OVERLAYAZ_REF_IDS; i++)
            g_object_unref(ui_map->pixbuf_ref[t][i]);
    for (i = 0; i < UI_VIEW_MAP_MARKER_CACHE; i++)
        if (ui_map->pixbuf_marker[i])
            g_object_unref(ui_map->pixbuf_marker[i]);

    g_slist_free(ui_map->img_marker);
    g_slist_free(ui_map->track_grid);
    g_slist_free(ui_map->track_marker);
    g_free(ui_map);
}

void
overlayaz_ui_view_map_sync(overlayaz_ui_view_map_t *ui_map,
                           gboolean                 active)
{
    /* TODO */
    //osm_gps_map_set_center(ui_map->map, lat, lon);
}

void
overlayaz_ui_view_map_update(overlayaz_ui_view_map_t *ui_map)
{
    struct overlayaz_location home;
    struct overlayaz_location ref;
    gint t, i;
    GSList *it;

    /* Clear the map */
    ui_view_map_remove_image(ui_map->map, &ui_map->img_home);

    for (t = 0; t < OVERLAYAZ_REF_TYPES; t++)
        for (i = 0; i < OVERLAYAZ_REF_IDS; i++)
            ui_view_map_remove_image(ui_map->map, &ui_map->img_ref[t][i]);

    for (it = ui_map->img_marker; it; it = it->next)
        osm_gps_map_image_remove(ui_map->map, it->data);
    g_slist_free(ui_map->img_marker);
    ui_map->img_marker = NULL;

    ui_view_map_remove_track(ui_map->map, &ui_map->track_bound[0]);
    ui_view_map_remove_track(ui_map->map, &ui_map->track_bound[1]);

    for (it = ui_map->track_grid; it; it = it->next)
        osm_gps_map_track_remove(ui_map->map, it->data);
    g_slist_free(ui_map->track_grid);
    ui_map->track_grid = NULL;

    for (it = ui_map->track_marker; it; it = it->next)
        osm_gps_map_track_remove(ui_map->map, it->data);
    g_slist_free(ui_map->track_marker);
    ui_map->track_marker = NULL;

    /* Recreate map objects */
    ui_view_map_update_grid(ui_map);
    ui_view_map_update_markers(ui_map);

    for (t = 0; t < OVERLAYAZ_REF_TYPES; t++)
    {
        for (i = 0; i < OVERLAYAZ_REF_IDS; i++)
        {
            if (overlayaz_get_ref_location(ui_map->o, t, i, &ref))
            {
                ui_map->img_ref[t][i] = osm_gps_map_image_add_with_alignment(ui_map->map,
                                                                             (gfloat)ref.latitude, (gfloat)ref.longitude,
                                                                             ui_map->pixbuf_ref[t][i],
                                                                             0.5f, 1.0f);
            }
        }
    }

    if (overlayaz_get_location(ui_map->o, &home))
    {
        ui_map->img_home = osm_gps_map_image_add_with_alignment(ui_map->map,
                                                                (gfloat)home.latitude, (gfloat)home.longitude,
                                                                ui_map->pixbuf_home,
                                                                0.5f, 1.0f);
    }
}

static void
ui_view_map_update_grid(overlayaz_ui_view_map_t *ui_map)
{
    struct overlayaz_location home;
    gdouble grid_distance;
    gdouble angle, step;
    gint i, count;
    OsmGpsMapTrack *track;

    if (!overlayaz_get_location(ui_map->o, &home))
        return;

    grid_distance = overlayaz_conf_get_map_grid_distance();

    if (overlayaz_grid_helper(ui_map->o, OVERLAYAZ_REF_AZ, &angle, &step, &count))
    {
        for (i = 0; i < count; i++)
        {
            track = ui_view_map_track_new(home.latitude, home.longitude, angle + i * step, grid_distance*1000, &color_grid);
            ui_map->track_grid = g_slist_append(ui_map->track_grid, track);
            osm_gps_map_track_add(ui_map->map, track);
        }
    }

    if (overlayaz_get_angle(ui_map->o, OVERLAYAZ_REF_AZ, 0.0, &angle))
    {
        ui_map->track_bound[0] = ui_view_map_track_new(home.latitude, home.longitude, angle, grid_distance*1000, &color_bound);
        osm_gps_map_track_add(ui_map->map, ui_map->track_bound[0]);
    }

    if (overlayaz_get_angle(ui_map->o, OVERLAYAZ_REF_AZ, overlayaz_get_width(ui_map->o), &angle))
    {
        ui_map->track_bound[1] = ui_view_map_track_new(home.latitude, home.longitude, angle, grid_distance*1000, &color_bound);
        osm_gps_map_track_add(ui_map->map, ui_map->track_bound[1]);
    }
}

static void
ui_view_map_update_markers(overlayaz_ui_view_map_t *ui_map)
{
    struct overlayaz_location home;
    overlayaz_marker_iter_t *iter;
    const overlayaz_marker_t *m;
    gdouble angle, dist;
    OsmGpsMapTrack *track;
    OsmGpsMapImage *image;
    gint pixbuf_id;

    if (!overlayaz_get_location(ui_map->o, &home))
        return;

    iter = overlayaz_marker_iter_new(overlayaz_get_marker_list(ui_map->o), &m);
    if (iter == NULL)
        return;

    do
    {
        if (overlayaz_marker_get_active(m))
        {
            overlayaz_geo_inverse(home.latitude, home.longitude,
                                  overlayaz_marker_get_latitude(m),
                                  overlayaz_marker_get_longitude(m),
                                  &angle, NULL, &dist);

            pixbuf_id = overlayaz_marker_iter_get_id(iter);
            if (pixbuf_id >= UI_VIEW_MAP_MARKER_CACHE)
                pixbuf_id = 0;

            if (!ui_map->pixbuf_marker[pixbuf_id])
                ui_map->pixbuf_marker[pixbuf_id] = overlayaz_icon_marker(UI_VIEW_MAP_ICON_SIZE, pixbuf_id);

            image = osm_gps_map_image_add_with_alignment(ui_map->map,
                                                         (gfloat)overlayaz_marker_get_latitude(m),
                                                         (gfloat)overlayaz_marker_get_longitude(m),
                                                         ui_map->pixbuf_marker[pixbuf_id],
                                                         0.5f, 1.0f);
            ui_map->img_marker = g_slist_append(ui_map->img_marker, image);

            /* Draw path for valid markers only (within image bounds) */
            if (overlayaz_get_position(ui_map->o, OVERLAYAZ_REF_AZ, angle, NULL))
            {
                track = ui_view_map_track_new(home.latitude, home.longitude, angle, dist, &color_marker);
                ui_map->track_marker = g_slist_append(ui_map->track_marker, track);
                osm_gps_map_track_add(ui_map->map, track);
            }
        }
    } while (overlayaz_marker_iter_next(iter, &m));
    overlayaz_marker_iter_free(iter);
}

static void
ui_view_map_remove_image(OsmGpsMap       *map,
                         OsmGpsMapImage **image)
{
    if (image && *image)
    {
        osm_gps_map_image_remove(map, *image);
        *image = NULL;
    }
}

static void
ui_view_map_remove_track(OsmGpsMap       *map,
                         OsmGpsMapTrack **track)
{
    if (track && *track)
    {
        osm_gps_map_track_remove(map, *track);
        *track = NULL;
    }
}

static OsmGpsMapTrack*
ui_view_map_track_new(gdouble  home_lat,
                      gdouble  home_lon,
                      gdouble  azi,
                      gdouble  dist,
                      GdkRGBA *color)
{
    OsmGpsMapTrack *track;
    OsmGpsMapPoint point;
    gdouble next_lat, next_lon;
    gdouble step;
    gint steps, i;

    track = osm_gps_map_track_new();
    osm_gps_map_point_set_degrees(&point, (gfloat)home_lat, (gfloat)home_lon);
    osm_gps_map_track_add_point(track, &point);

    steps = (gint)ceil(dist / UI_VIEW_MAP_PATH_STEP);
    step = dist / (gdouble)steps;
    for (i = 1; i <= steps; i++)
    {
        overlayaz_geo_direct(home_lat, home_lon, azi, step*i, &next_lat, &next_lon);
        osm_gps_map_point_set_degrees(&point, (gfloat)next_lat, (gfloat)next_lon);
        osm_gps_map_track_add_point(track, &point);
    }

    if (color)
        osm_gps_map_track_set_color(track, color);

    return track;
}

static gboolean
ui_view_map_press(GtkWidget               *widget,
                  GdkEventButton          *event,
                  overlayaz_ui_view_map_t *ui_map)
{
    OsmGpsMapPoint coord;
    gfloat lat, lon;

    if (event->type != GDK_BUTTON_PRESS)
        return GDK_EVENT_PROPAGATE;

    if (event->button == GDK_BUTTON_PRIMARY)
        ui_map->map_busy = TRUE;
    else
    {
        osm_gps_map_convert_screen_to_geographic(ui_map->map, (gint)round(event->x), (gint)round(event->y), &coord);
        osm_gps_map_point_get_degrees(&coord, &lat, &lon);

        if (event->button == GDK_BUTTON_SECONDARY)
            overlayaz_ui_action_location(ui_map->ui, OVERLAYAZ_UI_ACTION_SET, lat, lon);
        else if (event->button == GDK_BUTTON_MIDDLE)
            overlayaz_ui_action_location(ui_map->ui, OVERLAYAZ_UI_ACTION_INFO, lat, lon);
    }

    return GDK_EVENT_PROPAGATE;
}

static gboolean
ui_view_map_scroll(GtkWidget               *widget,
                   GdkEventScroll          *event,
                   overlayaz_ui_view_map_t *ui_map)
{
    gint zoom;

    /* Workaround for bug in OsmGpsMap.
     * Do not allow zooming in/out the map while left mouse button is clicked. */
    if (ui_map->map_busy)
        return GDK_EVENT_STOP;

    g_object_get(ui_map->map, "zoom", &zoom, NULL);

    /* The lowest zoom levels are unwanted here. */
    if (event->direction == GDK_SCROLL_DOWN && zoom <= UI_VIEW_MAP_ZOOM_MIN)
        return GDK_EVENT_STOP;

    /* Workaround for another bug in OsmGpsMap.
     * The highest zoom level (20) produces corrupted tracks. */
    if (event->direction == GDK_SCROLL_UP && zoom >= UI_VIEW_MAP_ZOOM_MAX)
        return GDK_EVENT_STOP;

    return GDK_EVENT_PROPAGATE;
}

static gboolean
ui_view_map_release(GtkWidget               *widget,
                    GdkEventButton          *event,
                    overlayaz_ui_view_map_t *ui_map)
{
    if (event->type == GDK_BUTTON_RELEASE)
    {
        ui_map->map_busy = FALSE;

        /* Primary button is used to move the map. New screen-to-coordinates mappings are not
         * available yet in this case, but the previous measurement is still valid, so we will keep it. */
        if (event->button != GDK_BUTTON_PRIMARY)
            ui_view_map_measure(ui_map, event->x, event->y);
    }

    return GDK_EVENT_PROPAGATE;
}

static gboolean
ui_view_map_motion(GtkWidget               *widget,
                   GdkEventMotion          *event,
                   overlayaz_ui_view_map_t *ui_map)
{
    ui_view_map_measure(ui_map, event->x, event->y);
    return GDK_EVENT_PROPAGATE;
}

static gboolean
ui_view_map_leave(GtkWidget               *widget,
                  GdkEvent                *event,
                  overlayaz_ui_view_map_t *ui_map)
{
    overlayaz_ui_show_azimuth(ui_map->ui, NAN);
    overlayaz_ui_show_distance(ui_map->ui, NAN);
    return GDK_EVENT_PROPAGATE;
}

static void
ui_view_map_measure(overlayaz_ui_view_map_t *ui_map,
                    gdouble                  x,
                    gdouble                  y)
{
    OsmGpsMapPoint coord;
    struct overlayaz_location home;
    gdouble angle, dist;
    gfloat lat, lon;

    if (ui_map->map_busy)
        return;

    osm_gps_map_convert_screen_to_geographic(ui_map->map, (gint)round(x), (gint)round(y), &coord);
    osm_gps_map_point_get_degrees(&coord, &lat, &lon);

    if (overlayaz_get_location(ui_map->o, &home))
    {
        overlayaz_geo_inverse(home.latitude, home.longitude, lat, lon, &angle, NULL, &dist);
        overlayaz_ui_show_azimuth(ui_map->ui, angle);
        overlayaz_ui_show_distance(ui_map->ui, dist);
    }
    else
    {
        overlayaz_ui_show_azimuth(ui_map->ui, NAN);
        overlayaz_ui_show_distance(ui_map->ui, NAN);
    }
}
