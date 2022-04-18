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
#include <windows.h>

#define MINGW_FONT_FILE ".\\share\\fonts\\TTF\\DejaVuSansMono.ttf"
#define MINGW_QUICK_EDIT 0x40

static gint mingw_font;


void
mingw_init(void)
{
    HANDLE consoleHandle;
    DWORD consoleMode;

    mingw_font = AddFontResourceEx(MINGW_FONT_FILE, FR_PRIVATE, NULL);

    /* Disable the console quick edit feature (debug build) */
    consoleHandle = GetStdHandle(STD_INPUT_HANDLE);
    if(consoleHandle)
    {
        if(GetConsoleMode(consoleHandle, &consoleMode))
        {
            consoleMode &= ~MINGW_QUICK_EDIT;
            SetConsoleMode(consoleHandle, consoleMode);
        }
    }
}

void
mingw_cleanup(void)
{
    if(mingw_font)
        RemoveFontResourceEx(MINGW_FONT_FILE, FR_PRIVATE, NULL);
}

gboolean
mingw_uri_signal(GtkWidget *label,
                 gchar     *uri,
                 gpointer   data)
{
    ShellExecute(0, "open", uri, NULL, NULL, 1);
    return TRUE;
}
