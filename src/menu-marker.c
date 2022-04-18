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
#include "ui-util.h"

#define MARKER_TICK_NONE   "tick-none"
#define MARKER_TICK_TOP    "tick-top"
#define MARKER_TICK_BOTTOM "tick-bottom"


GtkWidget*
overlayaz_menu_marker(struct overlayaz_menu_marker *m)
{
    gint grid_pos = 0;

    m->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, OVERLAYAZ_WINDOW_GRID_SPACING);

    m->box_marker = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(m->box), m->box_marker, FALSE, FALSE, 0);

    m->button_down = gtk_button_new_from_icon_name("go-down-symbolic", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_box_pack_start(GTK_BOX(m->box_marker), m->button_down, FALSE, FALSE, 0);

    m->button_up = gtk_button_new_from_icon_name("go-up-symbolic", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_box_pack_start(GTK_BOX(m->box_marker), m->button_up, FALSE, FALSE, 0);

    m->combo_marker = gtk_combo_box_new();
    gtk_box_pack_start(GTK_BOX(m->box_marker), m->combo_marker, TRUE, TRUE, OVERLAYAZ_WINDOW_GRID_SPACING);

    m->renderer_marker = gtk_cell_renderer_text_new();
    gtk_cell_renderer_text_set_fixed_height_from_font(GTK_CELL_RENDERER_TEXT(m->renderer_marker), 1);
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(m->combo_marker), m->renderer_marker, FALSE);

    m->button_remove = gtk_button_new_from_icon_name("list-remove-symbolic", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_box_pack_start(GTK_BOX(m->box_marker), m->button_remove, FALSE, FALSE, 0);

    m->button_clear = gtk_button_new_from_icon_name("edit-clear-symbolic", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_box_pack_start(GTK_BOX(m->box_marker), m->button_clear, FALSE, FALSE, 0);

    m->grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(m->grid), OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_grid_set_column_spacing(GTK_GRID(m->grid), 2*OVERLAYAZ_WINDOW_GRID_SPACING);
    gtk_grid_set_row_homogeneous(GTK_GRID(m->grid), FALSE);
    gtk_box_pack_start(GTK_BOX(m->box), m->grid, FALSE, FALSE, 0);

    m->label_name = gtk_label_new("Name:");
    gtk_widget_set_valign(GTK_WIDGET(m->label_name), GTK_ALIGN_CENTER);
    gtk_widget_set_halign(GTK_WIDGET(m->label_name), GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(m->grid), m->label_name, 0, ++grid_pos, 2, 1);

    m->scrolled_name = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(m->scrolled_name), GTK_SHADOW_ETCHED_OUT);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(m->scrolled_name), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_grid_attach(GTK_GRID(m->grid), m->scrolled_name, 2, grid_pos, 1, 1);

    m->textview_name = gtk_text_view_new();
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(m->textview_name), 4);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(m->textview_name), 8);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(m->textview_name), 4);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(m->textview_name), 8);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(m->textview_name), GTK_WRAP_CHAR);
    gtk_container_add(GTK_CONTAINER(m->scrolled_name), m->textview_name);

    m->label_lat = gtk_label_new("Latitude [°]:");
    gtk_widget_set_halign(GTK_WIDGET(m->label_lat), GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(m->grid), m->label_lat, 0, ++grid_pos, 2, 1);

    m->spin_lat = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_LAT_MIN, OVERLAYAZ_WINDOW_LAT_MAX, OVERLAYAZ_WINDOW_LAT_STEP);
    gtk_grid_attach(GTK_GRID(m->grid), m->spin_lat, 2, grid_pos, 1, 1);

    m->label_lon = gtk_label_new("Longitude [°]:");
    gtk_widget_set_halign(GTK_WIDGET(m->label_lon), GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(m->grid), m->label_lon, 0, ++grid_pos, 2, 1);

    m->spin_lon = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_LON_MIN, OVERLAYAZ_WINDOW_LON_MAX, OVERLAYAZ_WINDOW_LON_STEP);
    gtk_grid_attach(GTK_GRID(m->grid), m->spin_lon, 2, grid_pos, 1, 1);

    m->label_azi = gtk_label_new("Azimuth [°]:");
    gtk_widget_set_halign(GTK_WIDGET(m->label_azi), GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(m->grid), m->label_azi, 0, ++grid_pos, 2, 1);

    m->spin_azi = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_AZI_MIN, OVERLAYAZ_WINDOW_AZI_MAX, OVERLAYAZ_WINDOW_AZI_STEP);
    gtk_grid_attach(GTK_GRID(m->grid), m->spin_azi, 2, grid_pos, 1, 1);

    m->label_dist = gtk_label_new("Distance [km]:");
    gtk_widget_set_halign(GTK_WIDGET(m->label_dist), GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(m->grid), m->label_dist, 0, ++grid_pos, 2, 1);

    m->spin_dist = gtk_spin_button_new_with_range(OVERLAYAZ_WINDOW_DIST_MIN, OVERLAYAZ_WINDOW_DIST_MAX, OVERLAYAZ_WINDOW_DIST_STEP);
    gtk_grid_attach(GTK_GRID(m->grid), m->spin_dist, 2, grid_pos, 1, 1);

    m->label_tick = gtk_label_new("Tick:");
    gtk_widget_set_halign(GTK_WIDGET(m->label_tick), GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(m->grid), m->label_tick, 0, ++grid_pos, 1, 1);

    m->button_tick_apply = gtk_button_new_from_icon_name("emblem-default-symbolic", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_widget_set_hexpand(m->button_tick_apply, FALSE);
    gtk_grid_attach(GTK_GRID(m->grid), m->button_tick_apply, 1, grid_pos, 1, 1);

    m->combo_tick = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(m->combo_tick), MARKER_TICK_NONE, "None");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(m->combo_tick), MARKER_TICK_TOP, "Top");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(m->combo_tick), MARKER_TICK_BOTTOM, "Bottom");
    gtk_grid_attach(GTK_GRID(m->grid), m->combo_tick, 2, grid_pos, 1, 1);

    m->label_font_marker = gtk_label_new("Font:");
    gtk_widget_set_halign(GTK_WIDGET(m->label_font_marker), GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(m->grid), m->label_font_marker, 0, ++grid_pos, 1, 1);

    m->button_font_apply = gtk_button_new_from_icon_name("emblem-default-symbolic", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_widget_set_hexpand(m->button_font_apply, FALSE);
    gtk_grid_attach(GTK_GRID(m->grid), m->button_font_apply, 1, grid_pos, 1, 1);

    m->box_font = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_grid_attach(GTK_GRID(m->grid), m->box_font, 2, grid_pos, 1, 1);

    m->font_marker = gtk_font_button_new();
    gtk_box_pack_start(GTK_BOX(m->box_font), m->font_marker, TRUE, TRUE, 0);

    m->color_marker_font = gtk_color_button_new();
    gtk_color_chooser_set_use_alpha(GTK_COLOR_CHOOSER(m->color_marker_font), TRUE);
    gtk_box_pack_start(GTK_BOX(m->box_font), m->color_marker_font, FALSE, FALSE, 0);

    m->label_pos = gtk_label_new("Position:");
    gtk_widget_set_halign(GTK_WIDGET(m->label_pos), GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(m->grid), m->label_pos, 0, ++grid_pos, 1, 1);

    m->button_pos_apply = gtk_button_new_from_icon_name("emblem-default-symbolic", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_widget_set_hexpand(m->button_pos_apply, FALSE);
    gtk_grid_attach(GTK_GRID(m->grid), m->button_pos_apply, 1, grid_pos, 1, 1);

    m->scale_pos = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, OVERLAYAZ_WINDOW_POS_MIN, OVERLAYAZ_WINDOW_POS_MAX, OVERLAYAZ_WINDOW_POS_STEP);
    gtk_widget_set_hexpand(m->scale_pos, TRUE);
    gtk_scale_set_value_pos(GTK_SCALE(m->scale_pos), GTK_POS_RIGHT);
    gtk_grid_attach(GTK_GRID(m->grid), m->scale_pos, 2, grid_pos, 1, 1);

    m->label_flags = gtk_label_new("Flags:");
    gtk_widget_set_halign(GTK_WIDGET(m->label_flags), GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(m->grid), m->label_flags, 0, ++grid_pos, 1, 1);

    m->button_active_apply = gtk_button_new_from_icon_name("emblem-default-symbolic", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_widget_set_hexpand(m->button_active_apply, FALSE);
    gtk_grid_attach(GTK_GRID(m->grid), m->button_active_apply, 1, grid_pos, 1, 1);

    m->check_active = gtk_check_button_new_with_label("Active");
    gtk_grid_attach(GTK_GRID(m->grid), m->check_active, 2, grid_pos, 1, 1);

    m->button_show_azi_apply = gtk_button_new_from_icon_name("emblem-default-symbolic", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_widget_set_hexpand(m->button_show_azi_apply, FALSE);
    gtk_grid_attach(GTK_GRID(m->grid), m->button_show_azi_apply, 1, ++grid_pos, 1, 1);

    m->check_show_azi = gtk_check_button_new_with_label("Show azimuth");
    gtk_grid_attach(GTK_GRID(m->grid), m->check_show_azi, 2, grid_pos, 1, 1);

    m->button_show_dist_apply = gtk_button_new_from_icon_name("emblem-default-symbolic", OVERLAYAZ_WINDOW_BUTTON_IMAGE);
    gtk_widget_set_hexpand(m->button_show_dist_apply, FALSE);
    gtk_grid_attach(GTK_GRID(m->grid), m->button_show_dist_apply, 1, ++grid_pos, 1, 1);

    m->check_show_dist = gtk_check_button_new_with_label("Show distance");
    gtk_grid_attach(GTK_GRID(m->grid), m->check_show_dist, 2, grid_pos, 1, 1);

    return m->box;
}
