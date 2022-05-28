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

#include <gexiv2/gexiv2.h>
#include "location.h"


gboolean
overlayaz_exif_get_location(const gchar               *filename,
                            struct overlayaz_location *location)
{
    GExiv2Metadata *metadata = gexiv2_metadata_new();

    if (!gexiv2_metadata_open_path(metadata, filename, NULL))
    {
        g_object_unref(metadata);
        return FALSE;
    }

#if GEXIV2_CHECK_VERSION(0, 12, 2)
    if (!gexiv2_metadata_try_get_gps_latitude(metadata, &location->latitude, NULL) ||
        !gexiv2_metadata_try_get_gps_longitude(metadata, &location->longitude, NULL))
#else
    if (!gexiv2_metadata_get_gps_latitude(metadata, &location->latitude) ||
        !gexiv2_metadata_get_gps_longitude(metadata, &location->longitude))
#endif
    {
        g_object_unref(metadata);
        return FALSE;
    }

    /* Altitude is optional */
#if GEXIV2_CHECK_VERSION(0, 12, 2)
    gexiv2_metadata_try_get_gps_altitude(metadata, &location->altitude, NULL);
#else
    gexiv2_metadata_get_gps_altitude(metadata, &location->altitude);
#endif

    g_object_unref(metadata);
    return TRUE;
}
