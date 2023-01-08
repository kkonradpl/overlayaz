/*
 *  overlayaz – photo visibility analysis software
 *  Copyright (c) 2020-2023  Konrad Kosmatka
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

#ifndef OVERLAYAZ_WINDOW_REFS_H_
#define OVERLAYAZ_WINDOW_REFS_H_
#include "overlayaz.h"

enum overlayaz_window_ref_mode
{
    OVERLAYAZ_REF_MODE_DISABLED = 0,
    OVERLAYAZ_REF_MODE_ONE_POINT = 1,
    OVERLAYAZ_REF_MODE_TWO_POINT = 2
};

enum overlayaz_window_ref_range
{
    OVERLAYAZ_REF_RANGE_LATITUDE = 0,
    OVERLAYAZ_REF_RANGE_LONGITUDE = 1,
    OVERLAYAZ_REF_RANGE_ALTITUDE = 2,
    OVERLAYAZ_REF_RANGE_POSITION = 3,
    OVERLAYAZ_REF_RANGE_ANGLE = 4,
    OVERLAYAZ_REF_RANGE_N
};

struct overlayaz_menu_reference
{
    GtkWidget *combo_mode;
    GtkWidget *label_azimuth;
    GtkWidget *label_position;
    GtkWidget *spin_ref[OVERLAYAZ_REF_IDS][OVERLAYAZ_REF_RANGE_N];
    GtkWidget *button_ref[OVERLAYAZ_REF_IDS];
    GtkWidget *label_altitude[OVERLAYAZ_REF_IDS];
    GtkWidget *button_altitude[OVERLAYAZ_REF_IDS];
    GtkWidget *box_ratio;
    GtkWidget *button_ratio_paste;
    GtkWidget *button_ratio_calc;
    GtkWidget *label_ratio;
    GtkWidget *spin_ratio;
};

struct overlayaz_menu_ref
{
    GtkWidget *box;
    GtkWidget *grid;
    GtkWidget *label_latitude;
    GtkWidget *label_longitude;
    GtkWidget *spin_latitude;
    GtkWidget *button_coord_ref;
    GtkWidget *spin_longitude;
    GtkWidget *box_location;
    GtkWidget *button_home;
    GtkWidget *button_exif;
    GtkWidget *label_altitude;
    GtkWidget *button_altitude;
    GtkWidget *spin_altitude;
    struct overlayaz_menu_reference refs[OVERLAYAZ_REF_TYPES];
};

GtkWidget* overlayaz_menu_ref(struct overlayaz_menu_ref*);

#endif
