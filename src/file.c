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
#include "file.h"
#include "profile.h"
#include "exif.h"

static GdkPixbuf* file_load_image(const gchar*);


enum overlayaz_file_load_error
overlayaz_file_load(overlayaz_t *o,
                    const gchar *filename)
{
    enum overlayaz_file_load_error ret = OVERLAYAZ_FILE_LOAD_OK;
    gchar *filename_image;
    gchar *filename_profile;
    GdkPixbuf *pixbuf;
    struct overlayaz_location location;

    overlayaz_reset(o);

    if (g_str_has_suffix(filename, OVERLAYAZ_EXTENSION_PROFILE))
    {
        filename_image = g_strndup(filename, strlen(filename) - strlen(OVERLAYAZ_EXTENSION_PROFILE));
        filename_profile = g_strdup(filename);
    }
    else
    {
        filename_image = g_strdup(filename);
        filename_profile = g_strdup_printf("%s%s", filename, OVERLAYAZ_EXTENSION_PROFILE);
    }

    pixbuf = file_load_image(filename_image);
    if (pixbuf == NULL)
    {
        ret = OVERLAYAZ_FILE_LOAD_ERROR_IMAGE_OPEN;
        goto overlayaz_file_load_cleanup;
    }

    if (!gdk_pixbuf_get_width(pixbuf) ||
        !gdk_pixbuf_get_height(pixbuf))
    {
        g_object_unref(pixbuf);
        ret = OVERLAYAZ_FILE_LOAD_ERROR_IMAGE_OPEN;
        goto overlayaz_file_load_cleanup;
    }

    overlayaz_set_filename(o, filename_image);
    overlayaz_set_pixbuf(o, pixbuf);
    overlayaz_unchanged(o);

    if (filename_profile &&
        g_file_test(filename_profile, G_FILE_TEST_EXISTS))
    {
        switch (overlayaz_profile_load(o, filename_profile))
        {
        case OVERLAYAZ_PROFILE_LOAD_OK:
            overlayaz_unchanged(o);
            break;

        case OVERLAYAZ_PROFILE_LOAD_ERROR_OPEN:
            ret = OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_OPEN;
            break;

        case OVERLAYAZ_PROFILE_LOAD_ERROR_PARSE:
            ret = OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_PARSE;
            break;

        case OVERLAYAZ_PROFILE_LOAD_ERROR_FORMAT:
            ret = OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_FORMAT;
            break;

        case OVERLAYAZ_PROFILE_LOAD_ERROR_VERSION:
            ret = OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_VERSION;
            break;
        }
    }
    else
    {
        /* Look up location in EXIF metadata if profile does not exist */
        overlayaz_exif_t *exif = overlayaz_exif_new(filename_image);
        if (exif)
        {
            if (overlayaz_exif_get_location(exif, &location))
            {
                overlayaz_set_location(o, &location);
                overlayaz_unchanged(o);
            }

            overlayaz_exif_free(exif);
        }
    }

overlayaz_file_load_cleanup:
    g_free(filename_image);
    g_free(filename_profile);
    return ret;
}

const gchar*
overlayaz_file_load_error(enum overlayaz_file_load_error error)
{
    switch (error)
    {
        case OVERLAYAZ_FILE_LOAD_OK:
            return "No error";
        case OVERLAYAZ_FILE_LOAD_ERROR_IMAGE_OPEN:
            return "Failed to open the image";
        case OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_OPEN:
            return "Failed to open the profile file";
        case OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_PARSE:
            return "Failed to parse the profile file";
        case OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_FORMAT:
            return "The profile file has an invalid format";
        case OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_VERSION:
            return "The profile format version is unsupported";
        default:
            return "Unknown error";
    }
}

static GdkPixbuf*
file_load_image(const gchar *filename)
{
    GdkPixbuf *pixbuf;
    GError *error = NULL;

    pixbuf = gdk_pixbuf_new_from_file(filename, &error);

    if (error)
    {
        g_warning("%s: %s", __func__, error->message);
        g_error_free(error);
    }

    return pixbuf;
}
