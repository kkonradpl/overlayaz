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
#include <math.h>
#include "geodesic/geodesic.h"

/* WGS84 */
#define OVERLAYAZ_GEO_A 6378137.0
#define OVERLAYAZ_GEO_F 1/298.257223563

static struct geod_geodesic g;

static void geodetic_to_geocentric(gdouble, gdouble, gdouble, gdouble*, gdouble*, gdouble*);


void
overlayaz_geo_init(void)
{
    geod_init(&g, OVERLAYAZ_GEO_A, OVERLAYAZ_GEO_F);
}

void
overlayaz_geo_direct(gdouble  lat1,
                     gdouble  lon1,
                     gdouble  azi1,
                     gdouble  dist,
                     gdouble *lat2,
                     gdouble *lon2)
{
    gdouble azi2;
    geod_direct(&g, lat1, lon1, azi1, dist, lat2, lon2, &azi2);
}

void
overlayaz_geo_inverse(gdouble  lat1,
                      gdouble  lon1,
                      gdouble  lat2,
                      gdouble  lon2,
                      gdouble *azi1,
                      gdouble *azi2,
                      gdouble *dist)
{
    gdouble a1, a2, s12;

    geod_inverse(&g, lat1, lon1, lat2, lon2, &s12, &a1, &a2);

    if (azi1)
    {
        if (a1 < 0.0)
            a1 += 360.0;
        *azi1 = a1;
    }

    if (azi2)
    {
        if (a2 < 0.0)
            a2 += 360.0;
        *azi2 = a2;
    }

    if (dist)
        *dist = s12;
}

void
overlayaz_geo_elevation(gdouble  lat1,
                        gdouble  lon1,
                        gdouble  alt1,
                        gdouble  lat2,
                        gdouble  lon2,
                        gdouble  alt2,
                        gdouble *elev)
{
    gdouble slat, clat, slon, clon;
    gdouble x1, y1, z1;
    gdouble x2, y2, z2;
    gdouble x, y, z;

    lat1 *= G_PI / 180.0;
    lon1 *= G_PI / 180.0;
    lat2 *= G_PI / 180.0;
    lon2 *= G_PI / 180.0;

    slat = sin(lat1);
    clat = cos(lat1);
    slon = sin(lon1);
    clon = cos(lon1);

    geodetic_to_geocentric(lat1, lon1, alt1, &x1, &y1, &z1);
    geodetic_to_geocentric(lat2, lon2, alt2, &x2, &y2, &z2);

    x2 -= x1;
    y2 -= y1;
    z2 -= z1;

    x = -slon * x2 + clon * y2;
    y = -clon * slat * x2 + -slon * slat * y2 + clat * z2;
    z = clon * clat * x2 + slon * clat * y2 + slat * z2;

    *elev = atan2(z, sqrt(x*x + y*y)) * 180.0 / G_PI;
}

static void
geodetic_to_geocentric(gdouble  lat,
                       gdouble  lon,
                       gdouble  alt,
                       gdouble *x,
                       gdouble *y,
                       gdouble *z)
{
    gdouble slat = sin(lat);
    gdouble clat = cos(lat);
    gdouble slon = sin(lon);
    gdouble clon = cos(lon);
    gdouble N = g.a / sqrt(1.0 - g.e2 * slat * slat);
    *x = (N + alt) * clat * clon;
    *y = (N + alt) * clat * slon;
    *z = ((N * (1.0 - g.e2)) + alt) * slat;
}
