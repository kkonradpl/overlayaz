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

#ifndef OVERLAYAZ_SRTM_H_
#define OVERLAYAZ_SRTM_H_

enum overlayaz_srtm_error
{
    OVERLAYAZ_SRTM_OK,
    OVERLAYAZ_SRTM_ERROR_DIR,
    OVERLAYAZ_SRTM_ERROR_MISSING,
    OVERLAYAZ_SRTM_ERROR_FORMAT,
    OVERLAYAZ_SRTM_ERROR_OPEN,
    OVERLAYAZ_SRTM_ERROR_READ,
    OVERLAYAZ_SRTM_ERROR_INVALID
};

gchar* overlayaz_srtm_filename(gdouble, gdouble);
enum overlayaz_srtm_error overlayaz_srtm_lookup(const gchar*, gdouble, gdouble, gdouble*);
enum overlayaz_srtm_error overlayaz_srtm_lookup_default(gdouble, gdouble, gdouble*);

#endif