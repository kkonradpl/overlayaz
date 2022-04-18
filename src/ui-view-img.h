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

#ifndef OVERLAYAZ_UI_VIEW_IMG_H_
#define OVERLAYAZ_UI_VIEW_IMG_H_

typedef struct overlayaz_ui_view_img overlayaz_ui_view_img_t;

overlayaz_ui_view_img_t* overlayaz_ui_view_img_new(overlayaz_ui_t*, GtkWidget*, const overlayaz_t*);
void overlayaz_ui_view_img_free(overlayaz_ui_view_img_t*);
void overlayaz_ui_view_img_sync(overlayaz_ui_view_img_t*, gboolean);
void overlayaz_ui_view_img_update(overlayaz_ui_view_img_t*);
void overlayaz_ui_view_img_zoom_fit(overlayaz_ui_view_img_t*);
void overlayaz_ui_view_img_zoom_in(overlayaz_ui_view_img_t*);

#endif
