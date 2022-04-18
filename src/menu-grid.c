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
#include "window.h"

#define GRID_STEP_MIN   0.01
#define GRID_STEP_MAX   100.0
#define GRID_STEP_STEP  0.01

#define GRID_WIDTH_MIN  0.0
#define GRID_WIDTH_MAX  1000.0
#define GRID_WIDTH_STEP 1.0


GtkWidget*
overlayaz_menu_grid(struct overlayaz_menu_grid *g)
{
    gint grid_pos = 0;

    g->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, OVERLAYAZ_WINDOW_GRID_SPACING);

    g->grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(g->grid), OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_grid_set_column_spacing(GTK_GRID(g->grid), 2*OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_grid_set_row_homogeneous(GTK_GRID(g->grid), TRUE);
    gtk_box_pack_start(GTK_BOX(g->box), g->grid, FALSE, FALSE, 0);

    g->label_grid = gtk_label_new("Grid:");
    gtk_widget_set_halign(GTK_WIDGET(g->label_grid), GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g->grid), g->label_grid, 0, grid_pos, 1, 1);

    g->check_azimuth = gtk_check_button_new_with_label("Azimuth");
    gtk_widget_set_halign(GTK_WIDGET(g->check_azimuth), GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(g->grid), g->check_azimuth, 1, grid_pos, 1, 1);

    g->check_elevation = gtk_check_button_new_with_label("Elevation");
    gtk_widget_set_halign(GTK_WIDGET(g->check_elevation), GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(g->grid), g->check_elevation, 2, grid_pos, 1, 1);

    g->label_step = gtk_label_new("Step:");
    gtk_widget_set_halign(GTK_WIDGET(g->label_step), GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g->grid), g->label_step, 0, ++grid_pos, 1, 1);

    g->spin_step_azimuth = gtk_spin_button_new_with_range(GRID_STEP_MIN, GRID_STEP_MAX, GRID_STEP_STEP);
    gtk_grid_attach(GTK_GRID(g->grid), g->spin_step_azimuth, 1, grid_pos, 1, 1);

    g->spin_step_elevation = gtk_spin_button_new_with_range(GRID_STEP_MIN, GRID_STEP_MAX, GRID_STEP_STEP);
    gtk_grid_attach(GTK_GRID(g->grid), g->spin_step_elevation, 2, grid_pos, 1, 1);

    g->label_position = gtk_label_new("Position:");
    gtk_widget_set_halign(GTK_WIDGET(g->label_position), GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g->grid), g->label_position, 0, ++grid_pos, 1, 1);

    g->scale_position_azimuth = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100, 0.1);
    gtk_scale_set_value_pos(GTK_SCALE(g->scale_position_azimuth), GTK_POS_RIGHT);
    gtk_grid_attach(GTK_GRID(g->grid), g->scale_position_azimuth, 1, grid_pos, 1, 1);

    g->scale_position_elevation = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, OVERLAYAZ_WINDOW_POS_MIN, OVERLAYAZ_WINDOW_POS_MAX, OVERLAYAZ_WINDOW_POS_STEP);
    gtk_scale_set_value_pos(GTK_SCALE(g->scale_position_elevation), GTK_POS_RIGHT);
    gtk_grid_attach(GTK_GRID(g->grid), g->scale_position_elevation, 2, grid_pos, 1, 1);

    g->label_width = gtk_label_new("Width:");
    gtk_widget_set_halign(GTK_WIDGET(g->label_width), GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g->grid), g->label_width, 0, ++grid_pos, 1, 1);

    g->box_width = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_grid_attach(GTK_GRID(g->grid), g->box_width, 1, grid_pos, 2, 1);

    g->spin_width = gtk_spin_button_new_with_range(GRID_WIDTH_MIN, GRID_WIDTH_MAX, GRID_WIDTH_STEP);
    gtk_widget_set_hexpand(g->spin_width, TRUE);
    gtk_box_pack_start(GTK_BOX(g->box_width), g->spin_width, TRUE, TRUE, 0);

    g->color = gtk_color_button_new();
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(g->color), TRUE);
    gtk_box_pack_start(GTK_BOX(g->box_width), g->color, FALSE, FALSE, 0);

    g->label_font = gtk_label_new("Font:");
    gtk_widget_set_halign(GTK_WIDGET(g->label_font), GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(g->grid), g->label_font, 0, ++grid_pos, 1, 1);

    g->box_font = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_grid_attach(GTK_GRID(g->grid), g->box_font, 1, grid_pos, 2, 1);

    g->font = gtk_font_button_new();
    gtk_widget_set_hexpand(g->font, FALSE);
    gtk_box_pack_start(GTK_BOX(g->box_font), g->font, TRUE, TRUE, 0);

    g->color_font = gtk_color_button_new();
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(g->color_font), TRUE);
    gtk_box_pack_start(GTK_BOX(g->box_font), g->color_font, FALSE, FALSE, 0);

    return g->box;
}
