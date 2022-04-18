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
#include "ui.h"
#include "ui-view-img.h"
#include "draw.h"
#include "geo.h"
#include "conf.h"

#define UI_VIEW_IMG_ZOOM_LIMIT 10.0
#define UI_VIEW_IMG_ZOOM_FACTOR 1.25
#define UI_VIEW_IMG_ROTATION_STEP 0.05

struct overlayaz_ui_view_img
{
    overlayaz_ui_t *ui;
    GtkDrawingArea *image;
    const overlayaz_t *o;
    cairo_surface_t *surface;
    gdouble scale;
    gdouble start_x;
    gdouble start_y;
    gdouble offset_x;
    gdouble offset_y;
    gboolean left_hold;
};

static gboolean ui_view_img_draw(GtkWidget*, cairo_t*, overlayaz_ui_view_img_t*);
static gboolean ui_view_img_click(GtkWidget*, GdkEventButton*, overlayaz_ui_view_img_t*);
static gboolean ui_view_img_motion(GtkWidget*, GdkEventMotion*, overlayaz_ui_view_img_t*);
static gboolean ui_view_img_scroll(GtkWidget*, GdkEventScroll*, overlayaz_ui_view_img_t*);
static gboolean ui_view_img_leave(GtkWidget*, GdkEvent*, overlayaz_ui_view_img_t*);

static cairo_surface_t* ui_view_img_cache_surface(overlayaz_ui_view_img_t*, cairo_surface_t*);

static void ui_view_img_measure(overlayaz_ui_view_img_t*, gdouble, gdouble);


overlayaz_ui_view_img_t*
overlayaz_ui_view_img_new(overlayaz_ui_t    *ui,
                          GtkWidget         *image,
                          const overlayaz_t *o)
{
    overlayaz_ui_view_img_t *ui_img = g_malloc0(sizeof(overlayaz_ui_view_img_t));

    ui_img->ui = ui;
    ui_img->image = GTK_DRAWING_AREA(image);
    ui_img->o = o;

    g_signal_connect(ui_img->image, "draw", G_CALLBACK(ui_view_img_draw), ui_img);
    g_signal_connect(ui_img->image, "button-press-event", G_CALLBACK(ui_view_img_click), ui_img);
    g_signal_connect(ui_img->image, "button-release-event", G_CALLBACK(ui_view_img_click), ui_img);
    g_signal_connect(ui_img->image, "motion-notify-event", G_CALLBACK(ui_view_img_motion), ui_img);
    g_signal_connect(ui_img->image, "leave-notify-event", G_CALLBACK(ui_view_img_leave), ui_img);
    g_signal_connect(ui_img->image, "scroll-event", G_CALLBACK(ui_view_img_scroll), ui_img);

    return ui_img;
}

void
overlayaz_ui_view_img_free(overlayaz_ui_view_img_t *ui_img)
{
    g_free(ui_img);
}

void
overlayaz_ui_view_img_sync(overlayaz_ui_view_img_t *ui_img,
                           gboolean                 active)
{
    /* Reset the scale, it will be recalculated during redraw */
    ui_img->scale = 0.0;

    if (ui_img->surface)
    {
        /* Remove cached surface */
        cairo_surface_destroy(ui_img->surface);
        ui_img->surface = NULL;
    }
}

void
overlayaz_ui_view_img_update(overlayaz_ui_view_img_t *ui_img)
{
    gtk_widget_queue_draw(GTK_WIDGET(ui_img->image));
}

void
overlayaz_ui_view_img_zoom_fit(overlayaz_ui_view_img_t *ui_img)
{
    /* Reset the scale, it will be recalculated during redraw */
    ui_img->scale = 0.0;
    overlayaz_ui_view_img_update(ui_img);
}

void
overlayaz_ui_view_img_zoom_in(overlayaz_ui_view_img_t *ui_img)
{
    gdouble img_x = gtk_widget_get_allocated_width(GTK_WIDGET(ui_img->image));
    gdouble img_y = gtk_widget_get_allocated_height(GTK_WIDGET(ui_img->image));
    gdouble pos_x = ui_img->offset_x + img_x/2.0 / ui_img->scale;
    gdouble pos_y = ui_img->offset_y + img_y/2.0 / ui_img->scale;

    /* Zoom in and recenter the viewport */
    ui_img->scale = 1.0;
    ui_img->offset_x = pos_x - img_x/2.0 / ui_img->scale;
    ui_img->offset_y = pos_y - img_y/2.0 / ui_img->scale;

    overlayaz_ui_view_img_update(ui_img);
}

static gboolean
ui_view_img_draw(GtkWidget               *widget,
                 cairo_t                 *cr,
                 overlayaz_ui_view_img_t *ui_img)
{
    const gint grid_pattern_len = 2;
    gdouble grid_pattern[grid_pattern_len];
    gint widget_width = gtk_widget_get_allocated_width(widget);
    gint widget_height = gtk_widget_get_allocated_height(widget);
    enum overlayaz_ref_type ref_type;
    enum overlayaz_ref_id ref_id;
    gboolean valid_ref;
    gboolean selected;
    gint t, i, j;
    gdouble pos;
    gdouble r, g, b;

    if (!overlayaz_get_pixbuf(ui_img->o))
        return GDK_EVENT_PROPAGATE;

    if (ui_img->surface == NULL)
        ui_img->surface = ui_view_img_cache_surface(ui_img, cairo_get_target(cr));

    gint pb_x = overlayaz_get_width(ui_img->o);
    gint pb_y = overlayaz_get_height(ui_img->o);

    /* Limit min scale level to the viewport size */
    if (widget_width > (gint)(pb_x*ui_img->scale) && widget_height > (gint)(pb_y*ui_img->scale))
        ui_img->scale = MIN(widget_width/(gfloat)pb_x, widget_height/(gfloat)pb_y);

    /* Keep the image boundaries */
    if (ui_img->offset_x < 0)
        ui_img->offset_x = 0;
    if (ui_img->offset_y < 0)
        ui_img->offset_y = 0;
    if (ui_img->offset_x > (pb_x - widget_width / ui_img->scale))
        ui_img->offset_x = pb_x - widget_width / ui_img->scale;
    if (ui_img->offset_y > (pb_y - widget_height / ui_img->scale))
        ui_img->offset_y = pb_y - widget_height / ui_img->scale;

    /* Center the image, if it is smaller than the viewport */
    if (widget_width >= pb_x*ui_img->scale)
        ui_img->offset_x = -(widget_width - pb_x*ui_img->scale) / 2.0 / ui_img->scale;
    if (widget_height >= pb_y*ui_img->scale)
        ui_img->offset_y = -(widget_height - pb_y*ui_img->scale) / 2.0 / ui_img->scale;

    cairo_save(cr);
    cairo_scale(cr, ui_img->scale, ui_img->scale);
    cairo_translate(cr, -ui_img->offset_x, -ui_img->offset_y);
    overlayaz_draw(cr, CAIRO_FILTER_BILINEAR, ui_img->surface, ui_img->o);
    cairo_restore(cr);

    valid_ref = overlayaz_ui_get_ref(ui_img->ui, &ref_type, &ref_id);

    for (t = 0; t < OVERLAYAZ_REF_TYPES; t++)
    {
        for (i = 0; i < OVERLAYAZ_REF_IDS; i++)
        {
            if (!overlayaz_get_ref_position(ui_img->o, t, i, &pos))
                break;

            selected = valid_ref && t == ref_type && i == ref_id;
            grid_pattern[0] = MAX(1.0*ui_img->scale, 1.0);
            grid_pattern[1] = grid_pattern[0];

            for (j = 0; j < 2; j++)
            {
                cairo_set_dash(cr, grid_pattern, grid_pattern_len, j*grid_pattern[0]);

                if (selected && j == 0)
                {
                    /* Highlight */
                    r = 1.0;
                    g = 0.0;
                    b = 0.0;
                }
                else
                {
                    /* Normal */
                    r = g = b = j;
                }

                cairo_set_source_rgba(cr, r, g, b, 0.75);

                cairo_set_line_width(cr, MAX(1.0*ui_img->scale, 1.0));
                if (t == OVERLAYAZ_REF_AZ)
                {
                    cairo_move_to(cr, (-ui_img->offset_x + pos) * ui_img->scale, 0);
                    cairo_line_to(cr, (-ui_img->offset_x + pos) * ui_img->scale, widget_height);
                }
                else
                {
                    cairo_move_to(cr, 0, (-ui_img->offset_y + pos) * ui_img->scale);
                    cairo_line_to(cr, widget_width,(-ui_img->offset_y + pos) * ui_img->scale);
                }
                cairo_stroke(cr);
            }
        }
    }

    return GDK_EVENT_PROPAGATE;
}

static gboolean
ui_view_img_click(GtkWidget               *widget,
                  GdkEventButton          *event,
                  overlayaz_ui_view_img_t *ui_img)
{
    gdouble pos_x, pos_y;

    if (event->type == GDK_BUTTON_PRESS)
    {
        if (event->button == GDK_BUTTON_PRIMARY)
        {
            ui_img->left_hold = TRUE;
            ui_img->start_x = event->x;
            ui_img->start_y = event->y;
            overlayaz_ui_view_img_update(ui_img);
            return GDK_EVENT_PROPAGATE;
        }

        if (ui_img->scale != 0.0)
        {
            pos_x = (event->x / ui_img->scale + ui_img->offset_x);
            pos_y = (event->y / ui_img->scale + ui_img->offset_y);

            if (event->button == GDK_BUTTON_SECONDARY)
            {
                overlayaz_ui_action_position(ui_img->ui, OVERLAYAZ_UI_ACTION_SET, pos_x, pos_y);
                ui_view_img_measure(ui_img, event->x, event->y);
            }
            else if (event->button == GDK_BUTTON_MIDDLE)
            {
                overlayaz_ui_action_position(ui_img->ui, OVERLAYAZ_UI_ACTION_INFO, pos_x, pos_y);
            }
        }
    }
    else if (event->type == GDK_BUTTON_RELEASE)
    {
        ui_img->left_hold = FALSE;
    }

    return GDK_EVENT_PROPAGATE;
}

static gboolean
ui_view_img_motion(GtkWidget               *widget,
                   GdkEventMotion          *event,
                   overlayaz_ui_view_img_t *ui_img)
{
    if (ui_img->left_hold)
    {
        ui_img->offset_x -= (event->x - ui_img->start_x)/ui_img->scale;
        ui_img->offset_y -= (event->y - ui_img->start_y)/ui_img->scale;
        ui_img->start_x = event->x;
        ui_img->start_y = event->y;
        overlayaz_ui_view_img_update(ui_img);
    }

    ui_view_img_measure(ui_img, event->x, event->y);
    return GDK_EVENT_PROPAGATE;
}

static gboolean
ui_view_img_scroll(GtkWidget               *widget,
                   GdkEventScroll          *event,
                   overlayaz_ui_view_img_t *ui_img)
{
    gdouble pos_x, pos_y;
    gdouble rotation;

    if (event->direction == GDK_SCROLL_UP)
    {
        if (event->state & GDK_CONTROL_MASK)
        {
            /* Rotate CW */
            rotation = overlayaz_get_rotation(ui_img->o);
            rotation += UI_VIEW_IMG_ROTATION_STEP / ui_img->scale;
            overlayaz_ui_set_rotation(ui_img->ui, rotation);
        }
        else
        {
            /* Zoom in */
            pos_x = ui_img->offset_x + event->x / ui_img->scale;
            pos_y = ui_img->offset_y + event->y / ui_img->scale;
            ui_img->scale *= UI_VIEW_IMG_ZOOM_FACTOR;
            if (ui_img->scale > UI_VIEW_IMG_ZOOM_LIMIT)
                ui_img->scale = UI_VIEW_IMG_ZOOM_LIMIT;
            ui_img->offset_x = pos_x - event->x / ui_img->scale;
            ui_img->offset_y = pos_y - event->y / ui_img->scale;
            overlayaz_ui_view_img_update(ui_img);
        }
    }
    else if (event->direction == GDK_SCROLL_DOWN)
    {
        if (event->state & GDK_CONTROL_MASK)
        {
            /* Rotate CCW */
            rotation = overlayaz_get_rotation(ui_img->o);
            rotation -= UI_VIEW_IMG_ROTATION_STEP / ui_img->scale;
            overlayaz_ui_set_rotation(ui_img->ui, rotation);
        }
        else
        {
            /* Zoom out */
            pos_x = ui_img->offset_x + event->x / ui_img->scale;
            pos_y = ui_img->offset_y + event->y / ui_img->scale;
            ui_img->scale /= UI_VIEW_IMG_ZOOM_FACTOR;
            ui_img->offset_x = pos_x - event->x / ui_img->scale;
            ui_img->offset_y = pos_y - event->y / ui_img->scale;
            overlayaz_ui_view_img_update(ui_img);
        }
    }

    return GDK_EVENT_PROPAGATE;
}

static gboolean
ui_view_img_leave(GtkWidget               *widget,
                  GdkEvent                *event,
                  overlayaz_ui_view_img_t *ui_img)
{
    overlayaz_ui_show_azimuth(ui_img->ui, NAN);
    overlayaz_ui_show_elevation(ui_img->ui, NAN);
    return GDK_EVENT_PROPAGATE;
}


static cairo_surface_t*
ui_view_img_cache_surface(overlayaz_ui_view_img_t *ui_img,
                          cairo_surface_t         *other)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_surface_create_similar(other,
                                           cairo_surface_get_content(other),
                                           overlayaz_get_width(ui_img->o),
                                           overlayaz_get_height(ui_img->o));

    cr = cairo_create(surface);
    gdk_cairo_set_source_pixbuf(cr, overlayaz_get_pixbuf(ui_img->o), 0, 0);
    cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_FAST);
    cairo_paint(cr);
    cairo_destroy(cr);
    return surface;
}

static void
ui_view_img_measure(overlayaz_ui_view_img_t *ui_img,
                    gdouble                  x,
                    gdouble                  y)
{
    gdouble angle;

    overlayaz_get_angle(ui_img->o, OVERLAYAZ_REF_AZ, x / ui_img->scale + ui_img->offset_x, &angle);
    overlayaz_ui_show_azimuth(ui_img->ui, angle);

    overlayaz_get_angle(ui_img->o, OVERLAYAZ_REF_EL, y / ui_img->scale + ui_img->offset_y, &angle);
    overlayaz_ui_show_elevation(ui_img->ui, angle);
}
