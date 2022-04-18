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

#ifndef OVERLAYAZ_WINDOW_H_
#define OVERLAYAZ_WINDOW_H_
#include "menu-ref.h"
#include "menu-grid.h"
#include "menu-marker.h"

#define OVERLAYAZ_WINDOW_BUTTON_IMAGE GTK_ICON_SIZE_SMALL_TOOLBAR
#define OVERLAYAZ_WINDOW_GRID_SPACING 2
#define OVERLAYAZ_WINDOW_DIALOG_MARGIN 10

#define OVERLAYAZ_WINDOW_AZI_MIN 0.0
#define OVERLAYAZ_WINDOW_AZI_MAX 360.0
#define OVERLAYAZ_WINDOW_AZI_STEP 0.001

#define OVERLAYAZ_WINDOW_LAT_MIN -90.0
#define OVERLAYAZ_WINDOW_LAT_MAX 90.0
#define OVERLAYAZ_WINDOW_LAT_STEP 0.000001

#define OVERLAYAZ_WINDOW_LON_MIN -180.0
#define OVERLAYAZ_WINDOW_LON_MAX 180.0
#define OVERLAYAZ_WINDOW_LON_STEP 0.000001

#define OVERLAYAZ_WINDOW_ALT_MIN -100000.0
#define OVERLAYAZ_WINDOW_ALT_MAX 100000.0
#define OVERLAYAZ_WINDOW_ALT_STEP 0.1

#define OVERLAYAZ_WINDOW_POS_MIN 0.0
#define OVERLAYAZ_WINDOW_POS_MAX 100.0
#define OVERLAYAZ_WINDOW_POS_STEP 0.01

#define OVERLAYAZ_WINDOW_DIST_MIN 0.0
#define OVERLAYAZ_WINDOW_DIST_MAX 10000.0
#define OVERLAYAZ_WINDOW_DIST_STEP 0.001

#define OVERLAYAZ_WINDOW_ROTATION_MIN -90.0
#define OVERLAYAZ_WINDOW_ROTATION_MAX 90.0
#define OVERLAYAZ_WINDOW_ROTATION_STEP 0.01

enum overlayaz_window_view
{
    OVERLAYAZ_WINDOW_VIEW_IMAGE = 0,
    OVERLAYAZ_WINDOW_VIEW_MAP = 1
};

enum overlayaz_window_menu
{
    OVERLAYAZ_WINDOW_MENU_REF = 0,
    OVERLAYAZ_WINDOW_MENU_GRID = 1,
    OVERLAYAZ_WINDOW_MENU_MARKER = 2
};

struct overlayaz_window
{
    GtkWidget *window;
    GtkWidget *box;

    /* MENU */
    GtkWidget *box_menu;
    GtkWidget *file_chooser;
    GtkWidget *notebook_menu;
    struct overlayaz_menu_ref r;
    struct overlayaz_menu_grid g;
    struct overlayaz_menu_marker m;

    GtkWidget *grid_meas;
    GtkWidget *label_meas_first;
    GtkWidget *label_meas_first_value;
    GtkWidget *label_meas_second;
    GtkWidget *label_meas_second_value;

    GtkWidget *box_bottom;
    GtkWidget *button_save;
    GtkWidget *button_preferences;
    GtkWidget *button_about;
    GtkWidget *button_export;

    /* VIEW */
    GtkWidget *notebook_view;
    GtkWidget *box_image;
    GtkWidget *image;
    GtkWidget *frame_rotation;
    GtkWidget *box_rotation;
    GtkWidget *label_rotation;
    GtkWidget *scale_rotation;
    GtkWidget *button_rotation_reset;
    GtkWidget *box_map;
    GtkWidget *map;
};

void overlayaz_window_init(struct overlayaz_window*);

#endif
