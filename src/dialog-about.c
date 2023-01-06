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

#include <gtk/gtk.h>
#include "overlayaz.h"
#ifdef G_OS_WIN32
#include "mingw.h"
#endif

#define APP_LICENCE \
"This program is free software; you can redistribute it and/or\n" \
"modify it under the terms of the GNU General Public License\n" \
"as published by the Free Software Foundation; either version 2\n" \
"of the License, or (at your option) any later version.\n" \
"\n" \
"This program is distributed in the hope that it will be useful,\n" \
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n" \
"GNU General Public License for more details.\n" \
"\n" \
"You should have received a copy of the GNU General Public License\n" \
"along with this program. If not, see http://www.gnu.org/licenses/\n"


void
overlayaz_dialog_about(GtkWindow *parent)
{
    GtkWidget *dialog;

    dialog = gtk_about_dialog_new();
#ifdef G_OS_WIN32
    g_signal_connect(dialog, "realize", G_CALLBACK(mingw_realize), NULL);
#endif
    gtk_window_set_icon_name(GTK_WINDOW(dialog), "gtk-about");
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), OVERLAYAZ_NAME);
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), OVERLAYAZ_VERSION);
    gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(dialog), OVERLAYAZ_ICON);
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "Copyright © " OVERLAYAZ_COPYRIGHT " Konrad Kosmatka");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "Photo visibility analysis software");
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(dialog), "http://fmdx.pl/overlayaz");
    gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(dialog), APP_LICENCE);
#ifdef G_OS_WIN32
    g_signal_connect(dialog, "activate-link", G_CALLBACK(mingw_uri_signal), NULL);
#endif
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
