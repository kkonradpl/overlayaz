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
#include <gdk/gdkwin32.h>
#include <windows.h>
#include <dwmapi.h>

#define MINGW_FONT_FILE ".\\share\\fonts\\TTF\\DejaVuSansMono.ttf"
#define MINGW_QUICK_EDIT 0x40

static gint mingw_font = 0;
static const char css_string[] =
"* {\n"
"    font-family: Sans;\n"
"    font-size: 10pt;\n"
"}\n";

static void mingw_dark_titlebar(GtkWidget*);


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

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css_string, -1, NULL);
    GdkScreen *screen = gdk_display_get_default_screen(gdk_display_get_default());
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
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

void
mingw_realize(GtkWidget *widget,
              gpointer   user_data)
{
    gboolean dark_theme = FALSE;

    g_object_get(gtk_settings_get_default(),
                 "gtk-application-prefer-dark-theme",
                 &dark_theme, NULL);

    if (dark_theme)
        mingw_dark_titlebar(widget);
}

static void
mingw_dark_titlebar(GtkWidget *widget)
{
    const DWORD dark_mode = 20;
    const DWORD dark_mode_pre20h1 = 19;
    const BOOL value = TRUE;
    GdkWindow *window = gtk_widget_get_window(widget);

    if (window == NULL)
        return;

    HWND handle = GDK_WINDOW_HWND(window);
    if (!SUCCEEDED(DwmSetWindowAttribute(handle, dark_mode, &value, sizeof(value))))
        DwmSetWindowAttribute(handle, dark_mode_pre20h1, &value, sizeof(value));
}
