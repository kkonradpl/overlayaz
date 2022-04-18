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
#include "font.h"

struct overlayaz_font
{
    gchar *name;
    PangoFontDescription *pango;
};


overlayaz_font_t*
overlayaz_font_new(const gchar *default_font)
{
    overlayaz_font_t *f = g_malloc0(sizeof(overlayaz_font_t));
    overlayaz_font_set(f, default_font);
    return f;
}

void
overlayaz_font_free(overlayaz_font_t *f)
{
    g_free(f->name);
    if (f->pango)
        pango_font_description_free(f->pango);
    g_free(f);
}

void
overlayaz_font_set(struct overlayaz_font *f,
                   const gchar           *font)
{
    g_free(f->name);
    f->name = g_strdup(font);
    if (f->pango)
        pango_font_description_free(f->pango);
    f->pango = pango_font_description_from_string(font);
}

const gchar*
overlayaz_font_get(const overlayaz_font_t *f)
{
    return f->name;
}

const PangoFontDescription*
overlayaz_font_get_pango(const overlayaz_font_t *f)
{
    return f->pango;
}
