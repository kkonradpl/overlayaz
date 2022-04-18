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

#ifndef OVERLAYAZ_MENU_MARKER_H_
#define OVERLAYAZ_MENU_MARKER_H_

struct overlayaz_menu_marker
{
    GtkWidget *box;
    GtkWidget *grid;
    GtkWidget *box_marker;
    GtkWidget *button_down;
    GtkWidget *button_up;
    GtkCellRenderer *renderer_marker;
    GtkWidget *combo_marker;
    GtkWidget *button_remove;
    GtkWidget *button_clear;
    GtkWidget *label_name;
    GtkWidget *scrolled_name;
    GtkWidget *textview_name;
    GtkWidget *label_lat;
    GtkWidget *spin_lat;
    GtkWidget *label_lon;
    GtkWidget *spin_lon;
    GtkWidget *label_azi;
    GtkWidget *spin_azi;
    GtkWidget *label_dist;
    GtkWidget *spin_dist;
    GtkWidget *label_tick;
    GtkWidget *combo_tick;
    GtkWidget *button_tick_apply;
    GtkWidget *label_font_marker;
    GtkWidget *button_font_apply;
    GtkWidget *box_font;
    GtkWidget *font_marker;
    GtkWidget *color_marker_font;
    GtkWidget *label_pos;
    GtkWidget *scale_pos;
    GtkWidget *button_pos_apply;
    GtkWidget *label_flags;
    GtkWidget *check_active;
    GtkWidget *button_active_apply;
    GtkWidget *check_show_azi;
    GtkWidget *button_show_azi_apply;
    GtkWidget *check_show_dist;
    GtkWidget *button_show_dist_apply;
};

GtkWidget* overlayaz_menu_marker(struct overlayaz_menu_marker*);

#endif
