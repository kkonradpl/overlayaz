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

#ifndef OVERLAYAZ_UI_UTIL_H_
#define OVERLAYAZ_UI_UTIL_H_

void overlayaz_ui_util_set_spin_button_text_visibility(GtkSpinButton*, gboolean);
gboolean overlayaz_ui_util_format_spin_button_zero(GtkSpinButton*, gpointer);

gchar* overlayaz_ui_util_format_distance(gdouble);
gchar* overlayaz_ui_util_format_angle(gdouble);

#endif
