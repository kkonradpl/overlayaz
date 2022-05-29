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

#ifndef OVERLAYAZ_CONF_H_
#define OVERLAYAZ_CONF_H_

#define OVERLAYAZ_CONF_IMAGE_FILTER_FAST     "fast"
#define OVERLAYAZ_CONF_IMAGE_FILTER_GOOD     "good"
#define OVERLAYAZ_CONF_IMAGE_FILTER_BEST     "best"
#define OVERLAYAZ_CONF_IMAGE_FILTER_NEAREST  "nearest"
#define OVERLAYAZ_CONF_IMAGE_FILTER_BILINEAR "bilinear"

void overlayaz_conf_init(const gchar*);
void overlayaz_conf_free(void);

gint overlayaz_conf_get_jpeg_quality(void);
gboolean overlayaz_conf_set_jpeg_quality(gint);

gchar* overlayaz_conf_get_image_filter(void);
gboolean overlayaz_conf_set_image_filter(const gchar*);

gchar* overlayaz_conf_get_open_path(void);
gboolean overlayaz_conf_set_open_path(const gchar*);

gchar* overlayaz_conf_get_export_path(void);
gboolean overlayaz_conf_set_export_path(const gchar*);

gchar* overlayaz_conf_get_srtm_path(void);
gboolean overlayaz_conf_set_srtm_path(const gchar*);

gint overlayaz_conf_get_map_source(void);
gboolean overlayaz_conf_set_map_source(gint);

gdouble overlayaz_conf_get_map_grid_distance(void);
gboolean overlayaz_conf_set_map_grid_distance(gdouble);

gdouble overlayaz_conf_get_latitude(void);
gboolean overlayaz_conf_set_latitude(gdouble);

gdouble overlayaz_conf_get_longitude(void);
gboolean overlayaz_conf_set_longitude(gdouble);

gdouble overlayaz_conf_get_altitude(void);
gboolean overlayaz_conf_set_altitude(gdouble);

gboolean overlayaz_conf_get_dark_theme(void);
gboolean overlayaz_conf_set_dark_theme(gboolean);

#endif
