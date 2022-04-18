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
#include "overlayaz.h"

#define OVERLAYAZ_ICON_COLOR_HOME_FILL       "ff4646"
#define OVERLAYAZ_ICON_COLOR_HOME_BORDER     "d73534"
#define OVERLAYAZ_ICON_COLOR_HOME_INTERNAL   "590000"

#define OVERLAYAZ_ICON_COLOR_REF_AZ_FILL     "46ff46"
#define OVERLAYAZ_ICON_COLOR_REF_AZ_BORDER   "35d734"
#define OVERLAYAZ_ICON_COLOR_REF_AZ_INTERNAL "000000"

#define OVERLAYAZ_ICON_COLOR_REF_EL_FILL     "ffff46"
#define OVERLAYAZ_ICON_COLOR_REF_EL_BORDER   "ffd734"
#define OVERLAYAZ_ICON_COLOR_REF_EL_INTERNAL "000000"

#define OVERLAYAZ_ICON_COLOR_MARKER_FILL     "4646ff"
#define OVERLAYAZ_ICON_COLOR_MARKER_BORDER   "3434d7"
#define OVERLAYAZ_ICON_COLOR_MARKER_INTERNAL "ffffff"

static GdkPixbuf* icon_draw(gint, const gchar*, const gchar*, const gchar*, const gchar*);

static const gchar icon_start[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n"
"<svg\n"
"   xmlns:svg=\"http://www.w3.org/2000/svg\"\n"
"   xmlns=\"http://www.w3.org/2000/svg\"\n"
"   width=\"5.6444445mm\"\n"
"   height=\"9.847393mm\"\n"
"   viewBox=\"0 0 20 34.892337\"\n"
"   id=\"markersvg\"\n"
"   version=\"1.1\">\n"
"  <g\n"
"     id=\"layer1\"\n"
"     transform=\"translate(-814.59595,-274.38623)\">\n"
"    <g\n"
"       id=\"marker\"\n"
"       transform=\"matrix(1.1855854,0,0,1.1855854,-151.17715,-57.3976)\">\n"
"      <path\n"
"         id=\"markerpath\"\n"
"         d=\"m 817.11249,282.97118 c -1.25816,1.34277 -2.04623,3.29881 -2.01563,5.13867 0.0639,3.84476 1.79693,5.3002 4.56836,10.59179 0.99832,2.32851 2.04027,4.79237 3.03125,8.87305 0.13772,0.60193 0.27203,1.16104 0.33416,1.20948 0.0621,0.0485 0.19644,-0.51262 0.33416,-1.11455 0.99098,-4.08068 2.03293,-6.54258 3.03125,-8.87109 2.77143,-5.29159 4.50444,-6.74704 4.56836,-10.5918 0.0306,-1.83986 -0.75942,-3.79785 -2.01758,-5.14062 -1.43724,-1.53389 -3.60504,-2.66908 -5.91619,-2.71655 -2.31115,-0.0475 -4.4809,1.08773 -5.91814,2.62162 z\"\n"
"         style=\"fill: #%s; stroke: #%s;\" />\n";

static const gchar icon_dot[] =
"      <circle\n"
"         r=\"3\"\n"
"         cy=\"288.25\"\n"
"         cx=\"823\"\n"
"         id=\"dot\"\n"
"         style=\"fill: #%s;\" />\n";

static const gchar icon_title[] =
"      <text\n"
"         alignment-baseline=\"middle\"\n"
"         text-anchor=\"middle\"\n"
"         style=\"font-family: monospace; font-size: %dpx; fill: #%s;\"\n"
"         x=\"823\"\n"
"         y=\"293\"\n"
"         id=\"title\">%s</text>\n";

static const gchar icon_end[] =
"    </g>\n"
"  </g>\n"
"</svg>";


GdkPixbuf*
overlayaz_icon_home(gint size)
{
    return icon_draw(size,
                     OVERLAYAZ_ICON_COLOR_HOME_FILL,
                     OVERLAYAZ_ICON_COLOR_HOME_BORDER,
                     OVERLAYAZ_ICON_COLOR_HOME_INTERNAL,
                     ".");
}

GdkPixbuf*
overlayaz_icon_ref(gint                    size,
                   enum overlayaz_ref_type type,
                   enum overlayaz_ref_id   id)
{
    return icon_draw(size,
                     (type == OVERLAYAZ_REF_AZ ? OVERLAYAZ_ICON_COLOR_REF_AZ_FILL : OVERLAYAZ_ICON_COLOR_REF_EL_FILL),
                     (type == OVERLAYAZ_REF_AZ ? OVERLAYAZ_ICON_COLOR_REF_AZ_BORDER : OVERLAYAZ_ICON_COLOR_REF_EL_BORDER),
                     (type == OVERLAYAZ_REF_AZ ? OVERLAYAZ_ICON_COLOR_REF_AZ_INTERNAL : OVERLAYAZ_ICON_COLOR_REF_EL_INTERNAL),
                     (id == OVERLAYAZ_REF_A ? "A" : "B"));
}

GdkPixbuf*
overlayaz_icon_marker(gint size,
                      gint id)
{
    gchar *text;

    if(id < 100)
        text = g_strdup_printf("%d", id);
    else
        text = g_strdup_printf(" ");

    GdkPixbuf *icon = icon_draw(size,
                                OVERLAYAZ_ICON_COLOR_MARKER_FILL,
                                OVERLAYAZ_ICON_COLOR_MARKER_BORDER,
                                OVERLAYAZ_ICON_COLOR_MARKER_INTERNAL,
                                text);
    g_free(text);
    return icon;
}

static GdkPixbuf*
icon_draw(gint         size,
          const gchar *c1,
          const gchar *c2,
          const gchar *c3,
          const gchar *title)
{
    GdkPixbufLoader *svg_loader;
    GdkPixbuf *pixbuf = NULL;
    GString *svg_string;
    gchar *svg;
    GError *err = NULL;
    gint font_size;

    font_size = (strlen(title) == 1) ? 13 : 11;

    svg_string = g_string_new(NULL);
    g_string_append_printf(svg_string, icon_start, c1, c2);
    if (g_strcmp0(title, ".") == 0)
        g_string_append_printf(svg_string, icon_dot, c3);
    else if (g_strcmp0(title, " ") != 0)
        g_string_append_printf(svg_string, icon_title, font_size, c3, title);
    g_string_append_printf(svg_string, icon_end);
    svg = g_string_free(svg_string, FALSE);

    svg_loader = gdk_pixbuf_loader_new();
    gdk_pixbuf_loader_set_size(svg_loader, (gint)round(size/1.74461685), size);
    gdk_pixbuf_loader_write(svg_loader, (guchar*)svg, strlen(svg), &err);
    if (gdk_pixbuf_loader_close(svg_loader, NULL))
    {
        pixbuf = gdk_pixbuf_loader_get_pixbuf(svg_loader);
        g_object_ref(pixbuf);
    }
    else
    {
        g_warning("%s: Failed to parse SVG image (title=%s)", __func__, title);
    }

    g_object_unref(svg_loader);
    g_free(svg);
    return pixbuf;
}

