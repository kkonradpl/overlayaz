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

#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdint.h>
#include <math.h>
#include "srtm.h"

#define SRTM_VALUE_SIZEOF 2

#define SRTM_GRID_3ARC 1201
#define SRTM_GRID_1ARC 3601

#define SRTM_SIZE_3ARC (SRTM_GRID_3ARC * SRTM_GRID_3ARC * SRTM_VALUE_SIZEOF)
#define SRTM_SIZE_1ARC (SRTM_GRID_1ARC * SRTM_GRID_1ARC * SRTM_VALUE_SIZEOF)

#define SRTM_INVALID -32768

static enum overlayaz_srtm_error srtm_parse(const gchar*, gdouble, gdouble, int16_t*);


gchar*
overlayaz_srtm_filename(gdouble latitude,
                        gdouble longitude)
{
    gint lat_int = abs((gint)floor(latitude));
    gint lon_int = abs((gint)floor(longitude));
    char lat_char = (latitude>=0) ? 'N' : 'S';
    char lon_char = (longitude>=0) ? 'E' : 'W';
    return g_strdup_printf("%c%02d%c%03d.hgt", lat_char, lat_int, lon_char, lon_int);
}

enum overlayaz_srtm_error
overlayaz_srtm_lookup(const gchar *directory,
                      gdouble      latitude,
                      gdouble      longitude,
                      gdouble     *out)
{
    GDir *dir;
    gchar *srtm_filename;
    const gchar *filename;
    gchar *path;
    int16_t value;
    enum overlayaz_srtm_error error;

    dir = g_dir_open(directory, 0, NULL);
    if (dir == NULL)
    {
        g_warning("%s: Failed to open directory %s", __func__, directory);
        return OVERLAYAZ_SRTM_ERROR_DIR;
    }

    srtm_filename = overlayaz_srtm_filename(latitude, longitude);
    error = OVERLAYAZ_SRTM_ERROR_MISSING;
    while ((filename = g_dir_read_name(dir)))
    {
        if (g_ascii_strcasecmp(srtm_filename, filename) == 0)
        {
            path = g_build_filename(directory, filename, NULL);
            error = srtm_parse(path, latitude, longitude, &value);
            g_free(path);
            break;
        }
    }

    if (error == OVERLAYAZ_SRTM_ERROR_MISSING)
        g_warning("%s: Missing file %s", __func__, srtm_filename);
    else if (error == OVERLAYAZ_SRTM_OK)
    {
        *out = value;
        if (value == SRTM_INVALID)
        {
            error = OVERLAYAZ_SRTM_ERROR_INVALID;
            g_warning("%s: Invalid value (%d)", __func__, SRTM_INVALID);
        }
    }

    g_free(srtm_filename);
    g_dir_close(dir);
    return error;
}

static enum overlayaz_srtm_error
srtm_parse(const gchar *path,
           gdouble      latitude,
           gdouble      longitude,
           int16_t     *out)
{
    FILE *fp;
    struct stat sb;
    gint grid, x, y;
    gint offset;
    uint8_t buffer[SRTM_VALUE_SIZEOF];

    if (stat(path, &sb) != 0)
    {
        g_warning("%s: Unable to stat %s", __func__, path);
        return OVERLAYAZ_SRTM_ERROR_OPEN;
    }

    switch (sb.st_size)
    {
    case SRTM_SIZE_3ARC:
        grid = SRTM_GRID_3ARC;
        break;
    case SRTM_SIZE_1ARC:
        grid = SRTM_GRID_1ARC;
        break;
    default:
        g_warning("%s: Unknown format of %s", __func__, path);
        return OVERLAYAZ_SRTM_ERROR_FORMAT;
    }

    fp = g_fopen(path, "r");
    if (fp == NULL)
    {
        g_warning("%s: Failed to open %s", __func__, path);
        return OVERLAYAZ_SRTM_ERROR_OPEN;
    }

    if (latitude < 0)
        latitude *= -1;
    if (longitude < 0)
        longitude *= -1;

    x = (gint)round((latitude - (gint)latitude) * (grid-1));
    y = (gint)round((longitude - (gint)longitude) * (grid-1));

    offset = ((grid-x-1)*grid + y) * SRTM_VALUE_SIZEOF;
    if (fseek(fp, offset, SEEK_SET) != 0)
    {
        g_warning("%s: Unable to fseek %s", __func__, path);
        fclose(fp);
        return OVERLAYAZ_SRTM_ERROR_READ;
    }

    if (fread(buffer, 1, SRTM_VALUE_SIZEOF, fp) != SRTM_VALUE_SIZEOF)
    {
        g_warning("%s: Unable to fread %s", __func__, path);
        fclose(fp);
        return OVERLAYAZ_SRTM_ERROR_READ;
    }

    *out = (int16_t) (buffer[0] << 8 | buffer[1]);
    fclose(fp);
    return OVERLAYAZ_SRTM_OK;
}
