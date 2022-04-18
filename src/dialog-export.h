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

#ifndef OVERLAYAZ_DIALOG_EXPORT_H_
#define OVERLAYAZ_DIALOG_EXPORT_H_

typedef struct overlayaz_dialog_export overlayaz_dialog_export_t;

overlayaz_dialog_export_t* overlayaz_dialog_export(GtkWindow*);
void overlayaz_dialog_export_free(overlayaz_dialog_export_t*);

const gchar* overlayaz_dialog_export_get_filename(overlayaz_dialog_export_t*);
gint overlayaz_dialog_export_get_quality(overlayaz_dialog_export_t*);
const gchar* overlayaz_dialog_export_get_filter(overlayaz_dialog_export_t*);

#endif
