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

    if (!gexiv2_metadata_try_get_gps_latitude(metadata, &location->latitude, NULL) ||
        !gexiv2_metadata_try_get_gps_longitude(metadata, &location->longitude, NULL))
    {
        g_object_unref(metadata);
        return FALSE;
    }

    /* Altitude is optional */
    gexiv2_metadata_try_get_gps_altitude(metadata, &location->altitude, NULL);

    g_object_unref(metadata);
    return TRUE;
}
