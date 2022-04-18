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

#ifndef OVERLAYAZ_UI_MENU_REF_H_
#define OVERLAYAZ_UI_MENU_REF_H_

typedef struct overlayaz_ui_menu_ref overlayaz_ui_menu_ref_t;

overlayaz_ui_menu_ref_t* overlayaz_ui_menu_ref_new(overlayaz_ui_t*, struct overlayaz_menu_ref*, overlayaz_t*);
void overlayaz_ui_menu_ref_free(overlayaz_ui_menu_ref_t*);
void overlayaz_ui_menu_ref_sync(overlayaz_ui_menu_ref_t*, gboolean);

void overlayaz_ui_menu_ref_none(overlayaz_ui_menu_ref_t*);
gboolean overlayaz_ui_menu_ref_get(overlayaz_ui_menu_ref_t*, enum overlayaz_ref_type*, enum overlayaz_ref_id*);

void overlayaz_ui_menu_ref_set_position(overlayaz_ui_menu_ref_t*, gdouble, gdouble);
void overlayaz_ui_menu_ref_set_location(overlayaz_ui_menu_ref_t*, gdouble, gdouble);

#endif
