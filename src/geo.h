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

#ifndef OVERLAYAZ_GEO_H_
#define OVERLAYAZ_GEO_H_

void overlayaz_geo_init(void);
void overlayaz_geo_direct(gdouble, gdouble, gdouble, gdouble, gdouble*, gdouble*);
void overlayaz_geo_inverse(gdouble, gdouble, gdouble, gdouble, gdouble*, gdouble*, gdouble*);
void overlayaz_geo_elevation(gdouble, gdouble, gdouble, gdouble, gdouble, gdouble, gdouble*);

#endif
