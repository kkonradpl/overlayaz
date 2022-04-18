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

#ifndef OVERLAYAZ_FONT_H_
#define OVERLAYAZ_FONT_H_

typedef struct overlayaz_font overlayaz_font_t;

overlayaz_font_t* overlayaz_font_new(const gchar*);
void overlayaz_font_free(overlayaz_font_t*);

void overlayaz_font_set(overlayaz_font_t*, const gchar*);
const gchar* overlayaz_font_get(const overlayaz_font_t*);
const PangoFontDescription* overlayaz_font_get_pango(const overlayaz_font_t*);

#endif

