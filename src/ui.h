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

#ifndef OVERLAYAZ_UI_H_
#define OVERLAYAZ_UI_H_
#include "overlayaz.h"
#include "window.h"

enum overlayaz_ui_update_mask
{
    OVERLAYAZ_UI_UPDATE_IMAGE = 1 << 0,
    OVERLAYAZ_UI_UPDATE_MAP   = 1 << 1
};

enum overlayaz_ui_action
{
    OVERLAYAZ_UI_ACTION_SET  = 0,
    OVERLAYAZ_UI_ACTION_INFO = 1
};

typedef struct overlayaz_ui overlayaz_ui_t;

void overlayaz_ui(overlayaz_t*);
GtkWindow* overlayaz_ui_get_parent(overlayaz_ui_t*);

void overlayaz_ui_update_view(overlayaz_ui_t*, enum overlayaz_ui_update_mask);

void overlayaz_ui_set_menu(overlayaz_ui_t*, enum overlayaz_window_menu);
enum overlayaz_window_menu overlayaz_ui_get_menu(overlayaz_ui_t *ui);
void overlayaz_ui_set_view(overlayaz_ui_t*, enum overlayaz_window_view);
enum overlayaz_window_view overlayaz_ui_get_view(overlayaz_ui_t *ui);

void overlayaz_ui_show_azimuth(overlayaz_ui_t*, gdouble);
void overlayaz_ui_show_elevation(overlayaz_ui_t*, gdouble);
void overlayaz_ui_show_distance(overlayaz_ui_t*, gdouble);

gboolean overlayaz_ui_get_ref(const overlayaz_ui_t*, enum overlayaz_ref_type*, enum overlayaz_ref_id*);

void overlayaz_ui_set_rotation(overlayaz_ui_t*, gdouble);

void overlayaz_ui_action_position(overlayaz_ui_t*, enum overlayaz_ui_action, gdouble, gdouble);
void overlayaz_ui_action_location(overlayaz_ui_t*, enum overlayaz_ui_action, gdouble, gdouble);

#endif
