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

#include <gtk/gtk.h>
#include "window.h"

static const gchar help_tutorial[] =
"1. Open a picture using:\n"
"a) file chooser button at the top of the left menu,\n"
"b) drag-and-drop operation.\n"
"\n"
"2. If necessary, rotate the image to level the horizon using:\n"
"a) rotation knob on the right side of the window,\n"
"b) mouse scroll while holding Ctrl key.\n"
"\n"
"3. Set the location where the photo was taken:\n"
"a) use the map view (click the right mouse button),\n"
"b) enter latitude and longitude values manually,\n"
"Altitude is optional for elevation angle calibration.\n"
"\n"
"4. Set azimuth (and elevation) references with:\n"
"a) right mouse button to select position (image)\n"
"b) right mouse button to select location (map).\n"
"\n"
"NOTE: Geodesic computations (distance and azimuth) are performed using GeographicLib which implements spheroid model. These algorithms provide the highest level of accuracy and produce different results than typical calculations (i.e. great-circle distances).";

static const gchar help_identification[] =
"1. Create a new marker using the image view (click the right mouse button). The path distance is the same as map grid distance set in preferences.\n"
"\n"
"2. Switch view to the map and follow the path from the photo location until you find something on its way.\n"
"\n"
"3. Create a new marker to the discovered object (right mouse button).\n"
"\n"
"4. Remove the marker created at the beginning.";

static const gchar help_tipsntricks[] =
"1. Make sure that lens correction is applied to the picture! Avoid wide angle lens (too much distortion).\n"
"\n"
"2. Use middle mouse button to get detailed information about image point or map coordinates.\n"
"\n"
"3. Store coordinates in preferences and recall them with home button. This is useful for dealing with multiple photos taken at the same location.\n"
"\n"
"4. Download SRTM data (HGT files) for your neighborhood and set path in preferences to automatically look up altitude (height above sea level).\n"
"\n"
"5. Take photos with location embedded in EXIF metadata. It will be entered automatically.\n"
"\n"
"6. Keyboard shortcuts:\n"
"a) Alt + 1 – switch to image view\n"
"b) Alt + 2 – switch to map view\n"
"c) Z – zoom image in (100%)\n"
"d) X – zoom image out (fit to window)";

static void menu_help_new(GtkWidget**, GtkWidget**, const gchar*);


GtkWidget*
overlayaz_menu_help(struct overlayaz_menu_help *h)
{
    h->box = gtk_box_new(GTK_ORIENTATION_VERTICAL, OVERLAYAZ_WINDOW_GRID_SPACING);

    h->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(h->stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(h->stack), 500);

    h->stack_switcher = gtk_stack_switcher_new();
    gtk_widget_set_hexpand(h->stack_switcher, TRUE);
    gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(h->stack_switcher), GTK_STACK(h->stack));

    gtk_box_pack_start(GTK_BOX(h->box), h->stack_switcher, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(h->box), h->stack, TRUE, TRUE, 0);

    menu_help_new(&h->scrolled_tutorial, &h->textview_tutorial, help_tutorial);
    gtk_stack_add_titled(GTK_STACK(h->stack), h->scrolled_tutorial, "tutorial", "Tutorial");

    menu_help_new(&h->scrolled_identification, &h->textview_identificaion, help_identification);
    gtk_stack_add_titled(GTK_STACK(h->stack), h->scrolled_identification, "id", "Identification");

    menu_help_new(&h->scrolled_tipsntricks, &h->textview_tipsntricks, help_tipsntricks);
    gtk_stack_add_titled(GTK_STACK(h->stack), h->scrolled_tipsntricks, "tipsntricks", "Tips & tricks");

    return h->box;
}

static void
menu_help_new(GtkWidget   **scrolled,
              GtkWidget   **textview,
              const gchar *text)
{
    *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(*scrolled), GTK_SHADOW_ETCHED_OUT);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(*scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    *textview = gtk_text_view_new();
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(*textview)), text, -1);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(*textview), FALSE);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(*textview), 4);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(*textview), 8);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(*textview), 4);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(*textview), 8);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(*textview), GTK_WRAP_CHAR);
    gtk_container_add(GTK_CONTAINER(*scrolled), *textview);
}
