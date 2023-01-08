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

#include <gexiv2/gexiv2.h>
#include "exif.h"

#define EXIF_FocalPlaneXResolution    "Exif.Photo.FocalPlaneXResolution"
#define EXIF_FocalPlaneYResolution    "Exif.Photo.FocalPlaneYResolution"
#define EXIF_FocalPlaneResolutionUnit "Exif.Photo.FocalPlaneResolutionUnit"

#define EXIF_UNIT_INCH       2
#define EXIF_UNIT_CENTIMETER 3

#define CM_TO_INCH_COEFF 2.54

struct overlayaz_exif
{
    GExiv2Metadata *metadata;
};

static gboolean exif_get_rational_value(overlayaz_exif_t*, const char*, gdouble*);


overlayaz_exif_t*
overlayaz_exif_new(const gchar *filename)
{
    GExiv2Metadata *metadata = gexiv2_metadata_new();
    overlayaz_exif_t *exif;

    if (!gexiv2_metadata_open_path(metadata, filename, NULL))
    {
        g_object_unref(metadata);
        return NULL;
    }

    if (!gexiv2_metadata_has_exif(metadata))
    {
        g_object_unref(metadata);
        return NULL;
    }

    exif = g_malloc0(sizeof(overlayaz_exif_t));
    exif->metadata = metadata;
    return exif;
}

void
overlayaz_exif_free(overlayaz_exif_t *exif)
{
    g_object_unref(exif->metadata);
    g_free(exif);
}

gboolean
overlayaz_exif_get_location(overlayaz_exif_t          *exif,
                            struct overlayaz_location *location)
{
#if GEXIV2_CHECK_VERSION(0, 12, 2)
    if (!gexiv2_metadata_try_get_gps_latitude(exif->metadata, &location->latitude, NULL) ||
        !gexiv2_metadata_try_get_gps_longitude(exif->metadata, &location->longitude, NULL))
#else
    if (!gexiv2_metadata_get_gps_latitude(exif->metadata, &location->latitude) ||
        !gexiv2_metadata_get_gps_longitude(exif->metadata, &location->longitude))
#endif
    {
        return FALSE;
    }

#if GEXIV2_CHECK_VERSION(0, 12, 2)
    if (!gexiv2_metadata_try_get_gps_altitude(exif->metadata, &location->altitude, NULL))
#else
    if (!gexiv2_metadata_get_gps_altitude(exif->metadata, &location->altitude))
#endif
    {
        /* Altitude is optional */
        location->altitude = 0.0;
    }

    return TRUE;
}

gboolean
overlayaz_exif_get_pixel_size_with_fallback(overlayaz_exif_t *exif,
                                            gint             *width,
                                            gint             *height)
{
#if GEXIV2_CHECK_VERSION(0, 12, 2)
    *width = gexiv2_metadata_try_get_metadata_pixel_width(exif->metadata, NULL);
    *height = gexiv2_metadata_try_get_metadata_pixel_height(exif->metadata, NULL);
#else
    *width = gexiv2_metadata_get_metadata_pixel_width(exif->metadata);
    *height = gexiv2_metadata_get_metadata_pixel_height(exif->metadata);
#endif

    if (*width > 0 && *height > 0)
        return TRUE;

    *width = gexiv2_metadata_get_pixel_width(exif->metadata);
    *height = gexiv2_metadata_get_pixel_height(exif->metadata);

    return (*width > 0 && *height > 0);
}

gboolean
overlayaz_exif_get_focal_length(overlayaz_exif_t *exif,
                                gdouble          *focal_length)
{
#if GEXIV2_CHECK_VERSION(0, 12, 2)
    *focal_length = gexiv2_metadata_try_get_focal_length(exif->metadata, NULL);
#else
    *focal_length = gexiv2_metadata_get_focal_length(exif->metadata);
#endif
    return (*focal_length > 0);
}

gboolean
overlayaz_exif_get_focal_plane_res(overlayaz_exif_t *exif,
                                   gdouble          *x_res,
                                   gdouble          *y_res)
{
    glong unit;

    if (!exif_get_rational_value(exif, EXIF_FocalPlaneXResolution, x_res))
        return FALSE;

    if (!exif_get_rational_value(exif, EXIF_FocalPlaneYResolution, y_res))
        return FALSE;

#if GEXIV2_CHECK_VERSION(0, 12, 2)
    unit = gexiv2_metadata_try_get_tag_long(exif->metadata, EXIF_FocalPlaneResolutionUnit, NULL);
#else
    unit = gexiv2_metadata_get_tag_long(exif->metadata, EXIF_FocalPlaneResolutionUnit);
#endif

    if (unit == EXIF_UNIT_INCH)
    {
        *x_res = *x_res / CM_TO_INCH_COEFF / 10.0;
        *y_res = *y_res / CM_TO_INCH_COEFF / 10.0;
        return TRUE;
    }
    else if (unit == EXIF_UNIT_CENTIMETER)
    {
        *x_res /= 10.0;
        *y_res /= 10.0;
        return TRUE;
    }

    return FALSE;
}

static gboolean
exif_get_rational_value(overlayaz_exif_t *exif,
                        const char       *tag,
                        gdouble          *out)
{
    gint nom, den;

#if GEXIV2_CHECK_VERSION(0, 12, 2)
    if (!gexiv2_metadata_try_get_exif_tag_rational(exif->metadata, tag, &nom, &den, NULL))
#else
    if (!gexiv2_metadata_get_exif_tag_rational(exif->metadata, tag, &nom, &den))
#endif
    {
        return FALSE;
    }

    *out = nom / (gdouble)den;
    return TRUE;
}
