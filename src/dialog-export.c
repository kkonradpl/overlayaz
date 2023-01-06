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
#include "dialog-export.h"
#include "conf.h"
#ifdef G_OS_WIN32
#include "mingw.h"
#endif

static void dialog_export_response(GtkWidget*, gint, gpointer);
static gboolean dialog_export_str_has_suffix(const gchar*, const gchar*);

struct overlayaz_dialog_export
{
    gchar *filename;
    gint quality;
    gchar *filter;
    GtkWidget *dialog;
    GtkWidget *box_extra;
    GtkWidget *label_quality;
    GtkWidget *spin_quality;
    GtkWidget *label_filter;
    GtkWidget *combo_filter;
};


overlayaz_dialog_export_t*
overlayaz_dialog_export(GtkWindow *parent)
{
    overlayaz_dialog_export_t *e;
    GtkFileFilter *file_filter;
    gchar *str;

    e = g_malloc0(sizeof(overlayaz_dialog_export_t));

    e->dialog = gtk_file_chooser_dialog_new("Export image",
                                            parent,
                                            GTK_FILE_CHOOSER_ACTION_SAVE,
                                            "Cancel", GTK_RESPONSE_CANCEL,
                                            "Save", GTK_RESPONSE_ACCEPT,
                                            NULL);
#ifdef G_OS_WIN32
    g_signal_connect(e->dialog, "realize", G_CALLBACK(mingw_realize), NULL);
#endif

    str = overlayaz_conf_get_export_path();
    if (str && strlen(str))
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(e->dialog), str);
    g_free(str);

    gtk_file_chooser_set_create_folders(GTK_FILE_CHOOSER(e->dialog), TRUE);
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(e->dialog), TRUE);

    file_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter, "JPEG image");
    gtk_file_filter_add_pattern(file_filter, "*" OVERLAYAZ_EXTENSION_IMAGE);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(e->dialog), file_filter);

    e->box_extra = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    e->label_quality = gtk_label_new("Quality:");
    gtk_box_pack_start(GTK_BOX(e->box_extra), e->label_quality, FALSE, FALSE, 0);

    e->spin_quality = gtk_spin_button_new_with_range(0, 100, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(e->spin_quality), overlayaz_conf_get_jpeg_quality());
    gtk_box_pack_start(GTK_BOX(e->box_extra), e->spin_quality, FALSE, FALSE, 0);

    e->label_filter = gtk_label_new("Filter:");
    gtk_box_pack_start(GTK_BOX(e->box_extra), e->label_filter, FALSE, FALSE, 0);

    e->combo_filter = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(e->combo_filter), OVERLAYAZ_CONF_IMAGE_FILTER_FAST, "Fast");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(e->combo_filter), OVERLAYAZ_CONF_IMAGE_FILTER_GOOD, "Good");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(e->combo_filter), OVERLAYAZ_CONF_IMAGE_FILTER_BEST, "Best");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(e->combo_filter), OVERLAYAZ_CONF_IMAGE_FILTER_NEAREST, "Nearest");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(e->combo_filter), OVERLAYAZ_CONF_IMAGE_FILTER_BILINEAR, "Bilinear");
    gtk_box_pack_start(GTK_BOX(e->box_extra), e->combo_filter, FALSE, FALSE, 0);

    str = overlayaz_conf_get_image_filter();
    if (!gtk_combo_box_set_active_id(GTK_COMBO_BOX(e->combo_filter), str))
        gtk_combo_box_set_active_id(GTK_COMBO_BOX(e->combo_filter), OVERLAYAZ_CONF_IMAGE_FILTER_BEST);
    g_free(str);

    gtk_widget_show_all(e->box_extra);
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(e->dialog), e->box_extra);

    g_signal_connect(e->dialog, "response", G_CALLBACK(dialog_export_response), e);
    while (gtk_dialog_run(GTK_DIALOG(e->dialog)) != GTK_RESPONSE_NONE);

    if (e->filename)
    {
        str = g_path_get_dirname(e->filename);
        overlayaz_conf_set_export_path(str);
        g_free(str);
        overlayaz_conf_set_jpeg_quality(e->quality);
        overlayaz_conf_set_image_filter(e->filter);
    }

    return e;
}

void
overlayaz_dialog_export_free(overlayaz_dialog_export_t *e)
{
    if (e)
    {
        g_free(e->filename);
        g_free(e->filter);
        g_free(e);
    }
}

const gchar*
overlayaz_dialog_export_get_filename(overlayaz_dialog_export_t *e)
{
    return e->filename;
}

gint
overlayaz_dialog_export_get_quality(overlayaz_dialog_export_t *e)
{
    return e->quality;
}

const gchar*
overlayaz_dialog_export_get_filter(overlayaz_dialog_export_t *e)
{
    return e->filter;
}

static void
dialog_export_response(GtkWidget *dialog,
                       gint       response_id,
                       gpointer   user_data)
{
    overlayaz_dialog_export_t *e = (overlayaz_dialog_export_t*)user_data;
    gchar *filename;
    size_t new_length;

    if (response_id != GTK_RESPONSE_ACCEPT)
    {
        gtk_widget_destroy(dialog);
        return;
    }

    if (!(filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog))))
        return;

    if (!dialog_export_str_has_suffix(filename, OVERLAYAZ_EXTENSION_IMAGE))
    {
        new_length = strlen(filename) + strlen(OVERLAYAZ_EXTENSION_IMAGE) + 1;
        filename = (gchar*)g_realloc(filename, new_length);
        g_strlcat(filename, OVERLAYAZ_EXTENSION_IMAGE, new_length);

        /* After adding the suffix, the GTK should check whether we may overwrite something. */
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), filename);
        gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
        g_free(filename);
        return;
    }

    e->filename = filename;
    e->quality = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(e->spin_quality));
    e->filter = g_strdup(gtk_combo_box_get_active_id(GTK_COMBO_BOX(e->combo_filter)));
    gtk_widget_destroy(dialog);
}

static gboolean
dialog_export_str_has_suffix(const gchar *string,
                             const gchar *suffix)
{
    size_t string_len = strlen(string);
    size_t suffix_len = strlen(suffix);

    if (string_len < suffix_len)
        return FALSE;

    return g_ascii_strncasecmp(string + string_len - suffix_len, suffix, suffix_len) == 0;
}
