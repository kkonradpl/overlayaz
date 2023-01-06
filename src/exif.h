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

#ifndef OVERLAYAZ_EXIF_H_
#define OVERLAYAZ_EXIF_H_
#include "location.h"

typedef struct overlayaz_exif overlayaz_exif_t;

overlayaz_exif_t* overlayaz_exif_new(const gchar*);
void overlayaz_exif_free(overlayaz_exif_t*);

gboolean overlayaz_exif_get_location(overlayaz_exif_t*, struct overlayaz_location*);

#endif
