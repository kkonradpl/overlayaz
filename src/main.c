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
#include <stdlib.h>
#include <getopt.h>
#include "overlayaz.h"
#include "conf.h"
#include "geo.h"
#include "ui.h"
#include "file.h"
#include "export.h"
#ifdef G_OS_WIN32
#include "mingw.h"
#endif

struct overlayaz_arg
{
    const gchar *config_path;
    const gchar *input_filename;
    const gchar *output_filename;
    const gchar *output_filter;
    gint output_quality;
};

static struct overlayaz_arg args =
{
    .config_path = NULL,
    .input_filename = NULL,
    .output_filename = NULL,
    .output_filter = NULL,
    .output_quality = -1
};

static void
show_usage(void)
{
    printf("overlayaz " OVERLAYAZ_VERSION "\n");
    printf("usage: overlayaz [-c config] [-o output] [-f filter] [-q quality] input\n");
    printf("options:\n");
    printf("  -c  configuration file\n");
    printf("headless output mode:\n");
    printf("  -o  output export file\n");
    printf("  -o  override output filter (fast, good, best, nearest, bilinear)\n");
    printf("  -q  override output quality (0-100)\n");
}

static void
parse_args(gint   argc,
           gchar *argv[])
{
    gint c;
    gchar *ptr;

    while ((c = getopt(argc, argv, "hc:o:f:q:")) != -1)
    {
        switch (c)
        {
        case 'h':
            show_usage();
            exit(0);

        case 'c':
            args.config_path = optarg;
            break;

        case 'o':
            args.output_filename = optarg;
            break;

        case 'f':
            args.output_filter = optarg;
            break;

        case 'q':
            args.output_quality = (gint)g_ascii_strtoll(optarg, &ptr, 10);
            if (ptr == optarg || args.output_quality < 0 || args.output_quality > 100)
            {
                fprintf(stderr, "WARNING: Invalid quality given, using value from configuration.\n");
                args.output_quality = -1;
            }
            break;

        case '?':
            if (optopt == 'c')
            {
                fprintf(stderr, "WARNING: No configuration path given, using default.\n");
            }
            else if (optopt == 'o')
            {
                fprintf(stderr, "ERROR: No output filename given, nothing to do here.\n");
                exit(1);
            }
            else if (optopt == 'f')
            {
                fprintf(stderr, "WARNING: No filter given, using value from configuration.\n");
            }
            else if (optopt == 'q')
            {
                fprintf(stderr, "WARNING: No quality given, using value from configuration.\n");
            }
            break;

        default:
            break;
        }
    }

    if (optind == argc - 1)
        args.input_filename = argv[optind];
    else if (optind < argc - 1)
    {
        fprintf(stderr, "ERROR: Loading more than one file at once is not supported.\n");
        exit(1);
    }

    if (args.output_filename &&
        !args.input_filename)
    {
        fprintf(stderr, "ERROR: Headless output mode (-o) requires input filename, giving up.\n");
        exit(1);
    }

    if (!args.output_filename)
    {
        if (args.output_filter)
            fprintf(stderr, "WARNING: Output filter (-f) requires headless output mode (-o), ignoring.\n");
        if (args.output_quality != -1)
            fprintf(stderr, "WARNING: Output quality (-q) requires headless output mode (-o), ignoring.\n");
    }
}

int
main (int   argc,
      char *argv[])
{
    overlayaz_t *o;
    enum overlayaz_file_load_error error;

    gtk_disable_setlocale();
    gtk_init(&argc, &argv);
    parse_args(argc, argv);

#ifdef G_OS_WIN32
    mingw_init();
#endif

    overlayaz_conf_init(args.config_path);

    if (overlayaz_conf_get_dark_theme())
        g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme", TRUE, NULL);

    overlayaz_geo_init();
    o = overlayaz_new();

    if (args.input_filename)
    {
        error = overlayaz_file_load(o, args.input_filename);
        if (error != OVERLAYAZ_FILE_LOAD_OK)
        {
            fprintf(stderr, "ERROR: %s\n", overlayaz_file_load_error(error));
            if (args.output_filename)
                return 1;
        }

        if (args.output_filename)
        {
            /* Headless mode */
            if (args.output_quality < 0)
                args.output_quality = overlayaz_conf_get_jpeg_quality();

            if (!args.output_filter)
                args.output_filter = overlayaz_conf_get_image_filter();

            if (!overlayaz_export(o, args.output_filename, args.output_filter, args.output_quality))
            {
                fprintf(stderr, "ERROR: Failed to save file\n");
                return 1;
            }
            return 0;
        }
    }

    overlayaz_ui(o);
    gtk_main();

    overlayaz_free(o);
    overlayaz_conf_free();

#ifdef G_OS_WIN32
    mingw_cleanup();
#endif

    return 0;
}
