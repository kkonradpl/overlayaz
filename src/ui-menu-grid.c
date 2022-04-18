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
#include "ui.h"
#include "ui-menu-grid.h"

struct overlayaz_ui_menu_grid
{
    overlayaz_ui_t *ui;
    struct overlayaz_menu_grid *g;
    overlayaz_t *o;
    gboolean lock;
};

static void ui_menu_grid_check_azimuth_toggled(GtkToggleButton*, overlayaz_ui_menu_grid_t*);
static void ui_menu_grid_check_elevation_toggled(GtkToggleButton*, overlayaz_ui_menu_grid_t*);
static void ui_menu_grid_spin_step_azimuth_changed(GtkSpinButton*, overlayaz_ui_menu_grid_t*);
static void ui_menu_grid_spin_step_elevation_changed(GtkSpinButton*, overlayaz_ui_menu_grid_t*);
static void ui_menu_grid_scale_position_azimuth_changed(GtkRange*, overlayaz_ui_menu_grid_t*);
static void ui_menu_grid_scale_position_elevation_changed(GtkRange*, overlayaz_ui_menu_grid_t*);
static void ui_menu_grid_spin_width_changed(GtkSpinButton*, overlayaz_ui_menu_grid_t*);
static void ui_menu_grid_color_set(GtkColorButton*, overlayaz_ui_menu_grid_t*);
static void ui_menu_grid_font_set(GtkFontButton*, overlayaz_ui_menu_grid_t*);
static void ui_menu_grid_color_font_set(GtkColorButton*, overlayaz_ui_menu_grid_t*);


overlayaz_ui_menu_grid_t*
overlayaz_ui_menu_grid_new(overlayaz_ui_t             *ui,
                           struct overlayaz_menu_grid *g,
                           overlayaz_t                *o)
{
    overlayaz_ui_menu_grid_t *ui_g = g_malloc0(sizeof(overlayaz_ui_menu_grid_t));

    ui_g->ui = ui;
    ui_g->g = g;
    ui_g->o = o;

    g_signal_connect(ui_g->g->check_azimuth, "toggled", G_CALLBACK(ui_menu_grid_check_azimuth_toggled), ui_g);
    g_signal_connect(ui_g->g->check_elevation, "toggled", G_CALLBACK(ui_menu_grid_check_elevation_toggled), ui_g);
    g_signal_connect(ui_g->g->spin_step_azimuth, "value-changed", G_CALLBACK(ui_menu_grid_spin_step_azimuth_changed), ui_g);
    g_signal_connect(ui_g->g->spin_step_elevation, "value-changed", G_CALLBACK(ui_menu_grid_spin_step_elevation_changed), ui_g);
    g_signal_connect(ui_g->g->scale_position_azimuth, "value-changed", G_CALLBACK(ui_menu_grid_scale_position_azimuth_changed), ui_g);
    g_signal_connect(ui_g->g->scale_position_elevation, "value-changed", G_CALLBACK(ui_menu_grid_scale_position_elevation_changed), ui_g);
    g_signal_connect(ui_g->g->spin_width, "value-changed", G_CALLBACK(ui_menu_grid_spin_width_changed), ui_g);
    g_signal_connect(ui_g->g->color, "color-set", G_CALLBACK(ui_menu_grid_color_set), ui_g);
    g_signal_connect(ui_g->g->font, "font-set", G_CALLBACK(ui_menu_grid_font_set), ui_g);
    g_signal_connect(ui_g->g->color_font, "color-set", G_CALLBACK(ui_menu_grid_color_font_set), ui_g);
    
    return ui_g;
}

void
overlayaz_ui_menu_grid_free(overlayaz_ui_menu_grid_t *ui_g)
{
    g_free(ui_g);
}

void
overlayaz_ui_menu_grid_sync(overlayaz_ui_menu_grid_t *ui_g,
                            gboolean                  active)
{
    gtk_widget_set_sensitive(ui_g->g->check_azimuth, active);
    gtk_widget_set_sensitive(ui_g->g->check_elevation, active);
    gtk_widget_set_sensitive(ui_g->g->spin_step_azimuth, active);
    gtk_widget_set_sensitive(ui_g->g->spin_step_elevation, active);
    gtk_widget_set_sensitive(ui_g->g->scale_position_azimuth, active);
    gtk_widget_set_sensitive(ui_g->g->scale_position_elevation, active);
    gtk_widget_set_sensitive(ui_g->g->spin_width, active);
    gtk_widget_set_sensitive(ui_g->g->color, active);
    gtk_widget_set_sensitive(ui_g->g->font, active);
    gtk_widget_set_sensitive(ui_g->g->color_font, active);
    
    ui_g->lock = TRUE;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui_g->g->check_azimuth), overlayaz_get_grid(ui_g->o, OVERLAYAZ_REF_AZ));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ui_g->g->check_elevation), overlayaz_get_grid(ui_g->o, OVERLAYAZ_REF_EL));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_g->g->spin_step_azimuth), overlayaz_get_grid_step(ui_g->o, OVERLAYAZ_REF_AZ));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_g->g->spin_step_elevation), overlayaz_get_grid_step(ui_g->o, OVERLAYAZ_REF_EL));
    gtk_range_set_value(GTK_RANGE(ui_g->g->scale_position_azimuth), overlayaz_get_grid_position(ui_g->o, OVERLAYAZ_REF_AZ));
    gtk_range_set_value(GTK_RANGE(ui_g->g->scale_position_elevation), overlayaz_get_grid_position(ui_g->o, OVERLAYAZ_REF_EL));
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(ui_g->g->spin_width), overlayaz_get_grid_width(ui_g->o));
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(ui_g->g->color), overlayaz_get_grid_color(ui_g->o));
    gtk_font_chooser_set_font_desc(GTK_FONT_CHOOSER(ui_g->g->font), overlayaz_font_get_pango(overlayaz_get_grid_font(ui_g->o)));
    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(ui_g->g->color_font), overlayaz_get_grid_font_color(ui_g->o));
    ui_g->lock = FALSE;
}

static void
ui_menu_grid_check_azimuth_toggled(GtkToggleButton          *toggle_button,
                                   overlayaz_ui_menu_grid_t *ui_g)
{
    if (!ui_g->lock)
    {
        overlayaz_set_grid(ui_g->o, OVERLAYAZ_REF_AZ, gtk_toggle_button_get_active(toggle_button));
        overlayaz_ui_update_view(ui_g->ui, OVERLAYAZ_UI_UPDATE_IMAGE | OVERLAYAZ_UI_UPDATE_MAP);
    }
}

static void
ui_menu_grid_check_elevation_toggled(GtkToggleButton          *toggle_button,
                                     overlayaz_ui_menu_grid_t *ui_g)
{
    if (!ui_g->lock)
    {
        overlayaz_set_grid(ui_g->o, OVERLAYAZ_REF_EL, gtk_toggle_button_get_active(toggle_button));
        overlayaz_ui_update_view(ui_g->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
    }
}

static void
ui_menu_grid_spin_step_azimuth_changed(GtkSpinButton            *spin_button,
                                       overlayaz_ui_menu_grid_t *ui_g)
{
    if (!ui_g->lock)
    {
        overlayaz_set_grid_step(ui_g->o, OVERLAYAZ_REF_AZ, gtk_spin_button_get_value(spin_button));
        overlayaz_ui_update_view(ui_g->ui, OVERLAYAZ_UI_UPDATE_IMAGE | OVERLAYAZ_UI_UPDATE_MAP);
    }
}

static void
ui_menu_grid_spin_step_elevation_changed(GtkSpinButton            *spin_button,
                                         overlayaz_ui_menu_grid_t *ui_g)
{
    if (!ui_g->lock)
    {
        overlayaz_set_grid_step(ui_g->o, OVERLAYAZ_REF_EL, gtk_spin_button_get_value(spin_button));
        overlayaz_ui_update_view(ui_g->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
    }
}

static void
ui_menu_grid_scale_position_azimuth_changed(GtkRange                 *range,
                                            overlayaz_ui_menu_grid_t *ui_g)
{
    if (!ui_g->lock)
    {
        overlayaz_set_grid_position(ui_g->o, OVERLAYAZ_REF_AZ, gtk_range_get_value(range));
        overlayaz_ui_update_view(ui_g->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
    }
}

static void
ui_menu_grid_scale_position_elevation_changed(GtkRange                 *range,
                                              overlayaz_ui_menu_grid_t *ui_g)
{
    if (!ui_g->lock)
    {
        overlayaz_set_grid_position(ui_g->o, OVERLAYAZ_REF_EL, gtk_range_get_value(range));
        overlayaz_ui_update_view(ui_g->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
    }
}

static void
ui_menu_grid_spin_width_changed(GtkSpinButton            *spin_button,
                                overlayaz_ui_menu_grid_t *ui_g)
{
    if (!ui_g->lock)
    {
        overlayaz_set_grid_width(ui_g->o, gtk_spin_button_get_value(spin_button));
        overlayaz_ui_update_view(ui_g->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
    }
}

static void
ui_menu_grid_color_set(GtkColorButton           *widget,
                       overlayaz_ui_menu_grid_t *ui_g)
{
    GdkRGBA color;
    if (!ui_g->lock)
    {
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &color);
        overlayaz_set_grid_color(ui_g->o, &color);
        overlayaz_ui_update_view(ui_g->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
    }
}

static void
ui_menu_grid_font_set(GtkFontButton            *widget,
                      overlayaz_ui_menu_grid_t *ui_g)
{
    gchar *font;
    if (!ui_g->lock)
    {
        font = gtk_font_chooser_get_font(GTK_FONT_CHOOSER(widget));
        overlayaz_set_grid_font(ui_g->o, font);
        g_free(font);
        overlayaz_ui_update_view(ui_g->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
    }
}

static void
ui_menu_grid_color_font_set(GtkColorButton           *widget,
                            overlayaz_ui_menu_grid_t *ui_g)
{
    GdkRGBA color;
    if (!ui_g->lock)
    {
        gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(widget), &color);
        overlayaz_set_grid_font_color(ui_g->o, &color);
        overlayaz_ui_update_view(ui_g->ui, OVERLAYAZ_UI_UPDATE_IMAGE);
    }
}
