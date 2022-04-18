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
#include "overlayaz.h"
#include "draw.h"
#include "conf.h"

gboolean
overlayaz_export(const overlayaz_t *o,
                 const gchar       *filename,
                 const gchar       *filter_str,
                 guint              quality)
{
    gint width, height;
    cairo_t *cr;
    cairo_surface_t *target;
    GdkPixbuf *pixbuf;
    gboolean ret;
    cairo_filter_t filter;
    gchar *quality_str;

    if (!overlayaz_get_pixbuf(o))
        return FALSE;

    width = overlayaz_get_width(o);
    height = overlayaz_get_height(o);
    quality = MIN(quality, 100);

    if (g_strcmp0(filter_str, OVERLAYAZ_CONF_IMAGE_FILTER_FAST) == 0)
        filter = CAIRO_FILTER_FAST;
    else if (g_strcmp0(filter_str, OVERLAYAZ_CONF_IMAGE_FILTER_GOOD) == 0)
        filter = CAIRO_FILTER_GOOD;
    else if (g_strcmp0(filter_str, OVERLAYAZ_CONF_IMAGE_FILTER_NEAREST) == 0)
        filter = CAIRO_FILTER_NEAREST;
    else if (g_strcmp0(filter_str, OVERLAYAZ_CONF_IMAGE_FILTER_BILINEAR) == 0)
        filter = CAIRO_FILTER_BILINEAR;
    else /* default */
        filter = CAIRO_FILTER_BEST;

    target = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
    cr = cairo_create(target);
    overlayaz_draw(cr, filter, NULL, o);
    cairo_destroy(cr);

    pixbuf = gdk_pixbuf_get_from_surface(target, 0, 0, width, height);
    quality_str = g_strdup_printf("%d", quality);

    ret = gdk_pixbuf_save(pixbuf, filename, "jpeg", NULL, "quality", quality_str, NULL);

    g_free(quality_str);
    g_object_unref(pixbuf);
    cairo_surface_destroy(target);
    return ret;
}
