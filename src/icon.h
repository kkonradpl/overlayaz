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

#ifndef OVERLAYAZ_ICON_H_
#define OVERLAYAZ_ICON_H_
#include "overlayaz.h"

GdkPixbuf* overlayaz_icon_home(gint);
GdkPixbuf* overlayaz_icon_ref(gint, enum overlayaz_ref_type, enum overlayaz_ref_id);
GdkPixbuf* overlayaz_icon_marker(gint, gint);

#endif
