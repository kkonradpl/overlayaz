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

#ifndef OVERLAYAZ_DIALOG_H_
#define OVERLAYAZ_DIALOG_H_
#include "ui.h"

void overlayaz_dialog(GtkWindow*, GtkMessageType, const gchar*, const gchar*, ...);
gint overlayaz_dialog_ask_unsaved(GtkWindow*);
gint overlayaz_dialog_ask_overwrite(GtkWindow*);
gboolean overlayaz_dialog_ask_yesno(GtkWindow*, const gchar*, const gchar*);

#endif
