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

#ifndef OVERLAYAZ_H_
#define OVERLAYAZ_H_
#include "location.h"
#include "marker.h"
#include "font.h"

#define OVERLAYAZ_NAME "overlayaz"
#define OVERLAYAZ_VERSION "1.1-git"
#define OVERLAYAZ_COPYRIGHT "2020-2023"
#define OVERLAYAZ_ICON "overlayaz"

typedef struct overlayaz overlayaz_t;

enum overlayaz_ref_type
{
    OVERLAYAZ_REF_AZ = 0,
    OVERLAYAZ_REF_EL = 1,
    OVERLAYAZ_REF_TYPES = 2
};

enum overlayaz_ref_id
{
    OVERLAYAZ_REF_A = 0,
    OVERLAYAZ_REF_B = 1,
    OVERLAYAZ_REF_IDS = 2
};

/* We assume that the profile extension will be always in lowercase */
#define OVERLAYAZ_EXTENSION_PROFILE ".ovlz"
#define OVERLAYAZ_EXTENSION_IMAGE   ".jpg"

overlayaz_t* overlayaz_new(void);
void overlayaz_free(overlayaz_t*);
void overlayaz_reset(overlayaz_t*);
gboolean overlayaz_changed(const overlayaz_t*);
void overlayaz_unchanged(overlayaz_t*);

void overlayaz_set_filename(overlayaz_t*, const gchar*);
const gchar* overlayaz_get_filename(const overlayaz_t*);

void overlayaz_set_pixbuf(overlayaz_t*, GdkPixbuf*);
const GdkPixbuf* overlayaz_get_pixbuf(const overlayaz_t*);
gint overlayaz_get_width(const overlayaz_t*);
gint overlayaz_get_height(const overlayaz_t*);

void overlayaz_set_rotation(overlayaz_t*, gdouble);
gdouble overlayaz_get_rotation(const overlayaz_t*);

void overlayaz_set_location(overlayaz_t*, const struct overlayaz_location*);
gboolean overlayaz_get_location(const overlayaz_t*, struct overlayaz_location*);

void overlayaz_set_ref_none(overlayaz_t*, enum overlayaz_ref_type);
void overlayaz_set_ref_one(overlayaz_t*, enum overlayaz_ref_type, const struct overlayaz_location*, gdouble, gdouble);
void overlayaz_set_ref_two(overlayaz_t*, enum overlayaz_ref_type, const struct overlayaz_location*, gdouble, const struct overlayaz_location*, gdouble);

gboolean overlayaz_get_ref_location(const overlayaz_t*, enum overlayaz_ref_type, enum overlayaz_ref_id, struct overlayaz_location*);
gboolean overlayaz_get_ref_position(const overlayaz_t*, enum overlayaz_ref_type, enum overlayaz_ref_id, gdouble*);

gboolean overlayaz_get_ratio(const overlayaz_t*, enum overlayaz_ref_type, gdouble*);
gboolean overlayaz_get_angle(const overlayaz_t*, enum overlayaz_ref_type, gdouble, gdouble*);
gboolean overlayaz_get_position(const overlayaz_t*, enum overlayaz_ref_type, gdouble, gdouble*);

void overlayaz_set_grid(overlayaz_t*, enum overlayaz_ref_type, gboolean);
gboolean overlayaz_get_grid(const overlayaz_t*, enum overlayaz_ref_type);

void overlayaz_set_grid_step(overlayaz_t*, enum overlayaz_ref_type, gdouble);
gdouble overlayaz_get_grid_step(const overlayaz_t*, enum overlayaz_ref_type);

void overlayaz_set_grid_position(overlayaz_t*, enum overlayaz_ref_type, gdouble);
gdouble overlayaz_get_grid_position(const overlayaz_t*, enum overlayaz_ref_type);

void overlayaz_set_grid_width(overlayaz_t*, gdouble);
gdouble overlayaz_get_grid_width(const overlayaz_t*);

void overlayaz_set_grid_color(overlayaz_t*, const GdkRGBA*);
const GdkRGBA* overlayaz_get_grid_color(const overlayaz_t*);

void overlayaz_set_grid_font(overlayaz_t*, const gchar*);
const overlayaz_font_t* overlayaz_get_grid_font(const overlayaz_t*);

void overlayaz_set_grid_font_color(overlayaz_t*, const GdkRGBA*);
const GdkRGBA* overlayaz_get_grid_font_color(const overlayaz_t*);

GtkListStore* overlayaz_get_marker_list(const overlayaz_t*);

#endif
