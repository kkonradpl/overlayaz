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

#ifndef OVERLAYAZ_UI_MENU_GRID_H_
#define OVERLAYAZ_UI_MENU_GRID_H_

typedef struct overlayaz_ui_menu_grid overlayaz_ui_menu_grid_t;

overlayaz_ui_menu_grid_t* overlayaz_ui_menu_grid_new(overlayaz_ui_t*, struct overlayaz_menu_grid*, overlayaz_t*);
void overlayaz_ui_menu_grid_free(overlayaz_ui_menu_grid_t*);
void overlayaz_ui_menu_grid_sync(overlayaz_ui_menu_grid_t*, gboolean);

#endif
