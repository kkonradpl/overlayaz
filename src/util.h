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

#ifndef OVERLAYAZ_UTIL_H_
#define OVERLAYAZ_UTIL_H_

gboolean overlayaz_util_grid_calc(const overlayaz_t*, enum overlayaz_ref_type, gdouble*, gdouble*, gint*);
gdouble overlayaz_util_fov_calc(gdouble, gdouble);

#endif
