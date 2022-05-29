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

#include <glib.h>
#include <glib/gstdio.h>
#include <sqlite3.h>
#include "conf.h"

#define CONF_DIR  "overlayaz"
#define CONF_FILE "overlayaz.sqlite"
#define CONF_DB_TIMEOUT_MS 50

#define CONF_DEFAULT_JPEG_QUALITY      "95"
#define CONF_DEFAULT_IMAGE_FILTER      OVERLAYAZ_CONF_IMAGE_FILTER_BEST
#define CONF_DEFAULT_OPEN_PATH         ""
#define CONF_DEFAULT_EXPORT_PATH       ""
#define CONF_DEFAULT_SRTM_PATH         ""
#define CONF_DEFAULT_MAP_SOURCE        "9"
#define CONF_DEFAULT_MAP_GRID_DISTANCE "100.0"
#define CONF_DEFAULT_LATITUDE          "0.0"
#define CONF_DEFAULT_LONGITUDE         "0.0"
#define CONF_DEFAULT_ALTITUDE          "0.0"
#define CONF_DEFAULT_DARK_THEME        "1"

static const gchar key_jpeg_quality[] = "jpeg-quality";
static const gchar key_image_filter[] = "image-filter";
static const gchar key_open_path[] = "open-path";
static const gchar key_export_path[] = "export-path";
static const gchar key_srtm_path[] = "srtm-path";
static const gchar key_map_source[] = "map-source";
static const gchar key_map_grid_distance[] = "map-grid-distance";
static const gchar key_latitude[] = "latitude";
static const gchar key_longitude[] = "longitude";
static const gchar key_altitude[] = "altitude";
static const gchar key_dark_theme[] = "dark-theme";

static const gchar sql_init[] = "CREATE TABLE IF NOT EXISTS `config`(`key` TEXT PRIMARY KEY, `value` TEXT);";
static const gchar sql_write[] = "INSERT OR REPLACE INTO `config`(`key`, `value`) VALUES(?, ?);";
static const gchar sql_read[] = "SELECT `value` FROM `config` WHERE `key`=?;";

struct overlayaz_conf
{
    gchar *path;
    sqlite3 *db;
};

static struct overlayaz_conf conf;

static sqlite3_stmt* conf_query(const gchar*, const gchar*);
static gchar* conf_read_string(const gchar*, const gchar*);
static gboolean conf_write_string(const gchar*, const gchar*);
static gdouble conf_read_double(const gchar*, const gchar*);
static gboolean conf_write_double(const gchar*, gdouble);
static gint conf_read_int(const gchar*, const gchar*);
static gboolean conf_write_int(const gchar*, gint);


void
overlayaz_conf_init(const gchar *custom_path)
{
    gchar *directory;
    gchar *error = NULL;

    if (custom_path)
        conf.path = g_strdup(custom_path);
    else
    {
        directory = g_build_filename(g_get_user_config_dir(), CONF_DIR, NULL);
        g_mkdir(directory, 0700);
        conf.path = g_build_filename(directory, CONF_FILE, NULL);
        g_free(directory);
    }

    if (sqlite3_open(conf.path, &conf.db) != SQLITE_OK)
    {
        g_warning("%s: Failed to open: %s", __func__, conf.path);
        sqlite3_close(conf.db);
        conf.db = NULL;
        return;
    }

    if (sqlite3_exec(conf.db, sql_init, NULL, NULL, &error) != SQLITE_OK)
    {
        g_warning("%s: Failed to initialize: %s", __func__, error);
        sqlite3_free(error);
        sqlite3_close(conf.db);
        conf.db = NULL;
        return;
    }

    if (sqlite3_busy_timeout(conf.db, CONF_DB_TIMEOUT_MS) != SQLITE_OK)
        g_warning("%s: Failed to set sqlite3_busy_timeout", __func__);
}

void
overlayaz_conf_free(void)
{
    g_free(conf.path);
    sqlite3_close(conf.db);
}

gint
overlayaz_conf_get_jpeg_quality(void)
{
    return conf_read_int(key_jpeg_quality, CONF_DEFAULT_JPEG_QUALITY);
}

gboolean
overlayaz_conf_set_jpeg_quality(gint value)
{
    return conf_write_int(key_jpeg_quality, value);
}

gchar*
overlayaz_conf_get_image_filter(void)
{
    return conf_read_string(key_image_filter, CONF_DEFAULT_IMAGE_FILTER);
}

gboolean
overlayaz_conf_set_image_filter(const gchar *value)
{
    return conf_write_string(key_image_filter, value);
}

gchar*
overlayaz_conf_get_open_path(void)
{
    return conf_read_string(key_open_path, CONF_DEFAULT_OPEN_PATH);
}

gboolean
overlayaz_conf_set_open_path(const gchar *value)
{
    return conf_write_string(key_open_path, value);
}

gchar*
overlayaz_conf_get_export_path(void)
{
    return conf_read_string(key_export_path, CONF_DEFAULT_EXPORT_PATH);
}

gboolean
overlayaz_conf_set_export_path(const gchar *value)
{
    return conf_write_string(key_export_path, value);
}

gchar*
overlayaz_conf_get_srtm_path(void)
{
    return conf_read_string(key_srtm_path, CONF_DEFAULT_SRTM_PATH);
}

gboolean
overlayaz_conf_set_srtm_path(const gchar *value)
{
    return conf_write_string(key_srtm_path, value);
}

gint
overlayaz_conf_get_map_source(void)
{
    return conf_read_int(key_map_source, CONF_DEFAULT_MAP_SOURCE);
}

gboolean
overlayaz_conf_set_map_source(gint value)
{
    return conf_write_int(key_map_source, value);
}

gdouble
overlayaz_conf_get_map_grid_distance(void)
{
    return conf_read_double(key_map_grid_distance, CONF_DEFAULT_MAP_GRID_DISTANCE);
}

gboolean
overlayaz_conf_set_map_grid_distance(gdouble value)
{
    return conf_write_double(key_map_grid_distance, value);
}

gdouble
overlayaz_conf_get_latitude(void)
{
    return conf_read_double(key_latitude, CONF_DEFAULT_LATITUDE);
}

gboolean
overlayaz_conf_set_latitude(gdouble value)
{
    return conf_write_double(key_latitude, value);
}

gdouble
overlayaz_conf_get_longitude(void)
{
    return conf_read_double(key_longitude, CONF_DEFAULT_LONGITUDE);
}

gboolean
overlayaz_conf_set_longitude(gdouble value)
{
    return conf_write_double(key_longitude, value);
}

gdouble
overlayaz_conf_get_altitude(void)
{
    return conf_read_double(key_altitude, CONF_DEFAULT_ALTITUDE);
}

gboolean
overlayaz_conf_set_altitude(gdouble value)
{
    return conf_write_double(key_altitude, value);
}

gboolean
overlayaz_conf_get_dark_theme(void)
{
    return conf_read_int(key_dark_theme, CONF_DEFAULT_DARK_THEME);
}

gboolean
overlayaz_conf_set_dark_theme(gboolean value)
{
    return conf_write_double(key_dark_theme, value);
}

static sqlite3_stmt*
conf_query(const gchar *key,
           const gchar *value)
{
    sqlite3_stmt *res;
    const gchar* sql;

    if (conf.db == NULL)
        return NULL;

    /* If only key is present, we will perform read */
    /* If also value is present, we will perform write */
    sql = (value ? sql_write : sql_read);

    if (sqlite3_prepare_v2(conf.db, sql, -1, &res, NULL) != SQLITE_OK)
    {
        g_warning("%s: Error in sqlite3_prepare_v2", __func__);
        return NULL;
    }

    if (sqlite3_bind_text(res, 1, key, -1, NULL) != SQLITE_OK)
    {
        g_warning("%s: Error in first sqlite3_bind_text", __func__);
        sqlite3_finalize(res);
        return NULL;
    }

    if (value)
    {
        if (sqlite3_bind_text(res, 2, value, -1, NULL) != SQLITE_OK)
        {
            g_warning("%s: Error in second sqlite3_bind_text", __func__);
            sqlite3_finalize(res);
            return NULL;
        }
    }

    return res;
}

static gchar*
conf_read_string(const gchar *key,
                 const gchar *default_value)
{
    sqlite3_stmt *res = conf_query(key, NULL);
    gchar *out = NULL;
    gint rc;

    if (res)
    {
        rc = sqlite3_step(res);
        if (rc == SQLITE_ROW)
            out = g_strdup((const char*)sqlite3_column_text(res, 0));
        else if (rc != SQLITE_DONE)
            g_warning("%s: Error in sqlite3_step (%d)", __func__, rc);
        sqlite3_finalize(res);
    }

    /* If no key is found or error occurred,
     * the default value is used instead. */
    if (out == NULL)
        out = g_strdup(default_value);

    return out;
}

static gboolean
conf_write_string(const gchar *key,
                  const gchar *value)
{
    sqlite3_stmt *res = conf_query(key, value);
    gboolean ret = FALSE;
    gint rc;

    if (res)
    {
        rc = sqlite3_step(res);
        if (rc == SQLITE_DONE)
            ret = TRUE;
        else
            g_warning("%s: Error in sqlite3_step (%d)", __func__, rc);
        sqlite3_finalize(res);
    }

    return ret;
}

static gdouble
conf_read_double(const gchar *key,
                 const gchar *default_value)
{
    gchar *string = conf_read_string(key, default_value);
    gdouble value = g_ascii_strtod(string, NULL);
    g_free(string);
    return value;
}

static gboolean
conf_write_double(const gchar *key,
                  gdouble      value)
{
    gchar string[G_ASCII_DTOSTR_BUF_SIZE];
    g_ascii_dtostr(string, G_ASCII_DTOSTR_BUF_SIZE, value);
    return conf_write_string(key, string);
}

static gint
conf_read_int(const gchar *key,
              const gchar *default_value)
{
    gchar *string = conf_read_string(key, default_value);
    gint value = (gint)g_ascii_strtoll(string, NULL, 10);
    g_free(string);
    return value;
}

static gboolean
conf_write_int(const gchar *key,
               gint         value)
{
    gchar *string;
    gboolean ret;
    string = g_strdup_printf("%d", value);
    ret = conf_write_string(key, string);
    g_free(string);
    return ret;
}
