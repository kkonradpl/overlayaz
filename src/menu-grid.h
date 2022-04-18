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

#ifndef OVERLAYAZ_MENU_GRID_H_
#define OVERLAYAZ_MENU_GRID_H_

struct overlayaz_menu_grid
{
    GtkWidget *box;
    GtkWidget *grid;
    GtkWidget *box_width;
    GtkWidget *box_font;
    GtkWidget *label_grid;
    GtkWidget *check_azimuth;
    GtkWidget *check_elevation;
    GtkWidget *label_step;
    GtkWidget *spin_step_azimuth;
    GtkWidget *spin_step_elevation;
    GtkWidget *label_position;
    GtkWidget *scale_position_azimuth;
    GtkWidget *scale_position_elevation;
    GtkWidget *label_width;
    GtkWidget *spin_width;
    GtkWidget *color;
    GtkWidget *label_font;
    GtkWidget *font;
    GtkWidget *color_font;
};

GtkWidget* overlayaz_menu_grid(struct overlayaz_menu_grid*);

#endif
