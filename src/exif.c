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

struct overlayaz_exif
{
    GExiv2Metadata *metadata;
};


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
