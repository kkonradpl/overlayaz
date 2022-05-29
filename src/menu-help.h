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

#ifndef OVERLAYAZ_MENU_HELP_H_
#define OVERLAYAZ_MENU_HELP_H_

struct overlayaz_menu_help
{
    GtkWidget *box;
    GtkWidget *stack;
    GtkWidget *stack_switcher;
    GtkWidget *scrolled_tutorial;
    GtkWidget *textview_tutorial;
    GtkWidget *scrolled_identification;
    GtkWidget *textview_identificaion;
    GtkWidget *scrolled_tipsntricks;
    GtkWidget *textview_tipsntricks;
    GtkWidget *scrolled_shortcuts;
    GtkWidget *textview_shortcuts;
};

GtkWidget* overlayaz_menu_help(struct overlayaz_menu_help*);

#endif
