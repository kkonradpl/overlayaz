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
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdlib.h>
#include "overlayaz.h"
#include "marker-list.h"
#include "marker-iter.h"
#include "overlayaz-default.h"
#include "geo.h"

#define PIXBUF_WIDTH 1000
#define PIXBUF_HEIGHT 1000
#define LOCATION_LATITUDE 50.0
#define LOCATION_LONGITUDE 20.0
#define LOCATION_ALTITUDE 0.0


typedef struct {
    overlayaz_t *o;
} test_context_t;

static int
group_setup(void **state)
{
    test_context_t *ctx = malloc(sizeof(test_context_t));
    *state = ctx;
    return 0;
}

static int
group_teardown(void **state)
{
    test_context_t *ctx = *state;
    free(ctx);
    return 0;
}

static int
test_setup(void **state)
{
    test_context_t *ctx = *state;
    ctx->o = overlayaz_new();
    return 0;
}

static int
test_setup2(void **state)
{
    test_context_t *ctx = *state;
    test_setup(state);
    overlayaz_set_pixbuf(ctx->o, gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, PIXBUF_WIDTH, PIXBUF_HEIGHT));
    overlayaz_set_location(ctx->o, &(struct overlayaz_location){LOCATION_LATITUDE, LOCATION_LONGITUDE, LOCATION_ALTITUDE});
    overlayaz_unchanged(ctx->o);
    return 0;
}
/*
static int
test_setup3(void **state)
{
    test_context_t *ctx = *state;
    gdouble latitude = 50.05;
    gdouble longitude = 20.05;
    gdouble altitude = 100.0;
    gdouble position = 250.0;
    gdouble ratio = 100.0;

    test_setup2(state);
    overlayaz_set_ref_one(ctx->o, OVERLAYAZ_REF_AZ, latitude, longitude, altitude, position, ratio);
    overlayaz_set_ref_one(ctx->o, OVERLAYAZ_REF_EL, latitude, longitude, altitude, position, ratio);
    return 0;
}
*/
static int
test_teardown(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_free(ctx->o);
    return 0;
}

static void
helper_overlayaz_default_values(const overlayaz_t *o)
{
    GtkListStore *marker_list;
    overlayaz_marker_iter_t *marker_iter;
    const overlayaz_marker_t *marker;
    overlayaz_font_t *font;
    GdkRGBA color;
    gchar *color_str_A, *color_str_B;

    assert_null(overlayaz_get_pixbuf(o));
    assert_null(overlayaz_get_filename(o));
    assert_int_equal(overlayaz_get_width(o), 0);
    assert_int_equal(overlayaz_get_height(o), 0);
    assert_float_equal(overlayaz_get_rotation(o), OVERLAYAZ_DEFAULT_ROTATION, FLT_EPSILON);
    assert_float_equal(OVERLAYAZ_DEFAULT_ROTATION, 0.0, FLT_EPSILON);
    assert_false(overlayaz_get_location(o, NULL));
    assert_false(overlayaz_get_ref_position(o, OVERLAYAZ_REF_AZ, OVERLAYAZ_REF_A, NULL));
    assert_false(overlayaz_get_ref_position(o, OVERLAYAZ_REF_AZ, OVERLAYAZ_REF_B, NULL));
    assert_false(overlayaz_get_ref_location(o, OVERLAYAZ_REF_AZ, OVERLAYAZ_REF_A, NULL));
    assert_false(overlayaz_get_ref_location(o, OVERLAYAZ_REF_AZ, OVERLAYAZ_REF_B, NULL));
    assert_false(overlayaz_get_ref_position(o, OVERLAYAZ_REF_EL, OVERLAYAZ_REF_A, NULL));
    assert_false(overlayaz_get_ref_position(o, OVERLAYAZ_REF_EL, OVERLAYAZ_REF_B, NULL));
    assert_false(overlayaz_get_ref_location(o, OVERLAYAZ_REF_EL, OVERLAYAZ_REF_A, NULL));
    assert_false(overlayaz_get_ref_location(o, OVERLAYAZ_REF_EL, OVERLAYAZ_REF_B, NULL));
    assert_int_equal(overlayaz_get_grid(o, OVERLAYAZ_REF_AZ), OVERLAYAZ_DEFAULT_GRID_AZIMUTH);
    assert_int_equal(overlayaz_get_grid(o, OVERLAYAZ_REF_EL), OVERLAYAZ_DEFAULT_GRID_ELEVATION);
    assert_float_equal(overlayaz_get_grid_step(o, OVERLAYAZ_REF_AZ), OVERLAYAZ_DEFAULT_GRID_AZIMUTH_STEP, FLT_EPSILON);
    assert_float_equal(overlayaz_get_grid_step(o, OVERLAYAZ_REF_EL), OVERLAYAZ_DEFAULT_GRID_ELEVATION_STEP, FLT_EPSILON);
    assert_float_equal(overlayaz_get_grid_position(o, OVERLAYAZ_REF_AZ), OVERLAYAZ_DEFAULT_GRID_POSITION_AZIMUTH, FLT_EPSILON);
    assert_float_equal(overlayaz_get_grid_position(o, OVERLAYAZ_REF_EL), OVERLAYAZ_DEFAULT_GRID_POSITION_ELEVATION, FLT_EPSILON);
    assert_float_equal(overlayaz_get_grid_width(o), OVERLAYAZ_DEFAULT_GRID_WIDTH, FLT_EPSILON);

    gdk_rgba_parse(&color, OVERLAYAZ_DEFAULT_GRID_COLOR);
    color_str_A = gdk_rgba_to_string(&color);
    color_str_B = gdk_rgba_to_string(overlayaz_get_grid_color(o));
    assert_string_equal(color_str_A, color_str_B);
    g_free(color_str_A);
    g_free(color_str_B);

    assert_non_null(overlayaz_get_grid_font(o));
    font = overlayaz_font_new(OVERLAYAZ_DEFAULT_GRID_FONT);
    assert_string_equal(overlayaz_font_get(overlayaz_get_grid_font(o)), overlayaz_font_get(font));
    assert_non_null(overlayaz_font_get_pango(overlayaz_get_grid_font(o)));
    overlayaz_font_free(font);

    gdk_rgba_parse(&color, OVERLAYAZ_DEFAULT_GRID_FONT_COLOR);
    color_str_A = gdk_rgba_to_string(&color);
    color_str_B = gdk_rgba_to_string(overlayaz_get_grid_font_color(o));
    assert_string_equal(color_str_A, color_str_B);
    g_free(color_str_A);
    g_free(color_str_B);

    marker_list = overlayaz_get_marker_list(o);
    assert_non_null(marker_list);
    marker_iter = overlayaz_marker_iter_new(marker_list, &marker);
    assert_null(marker_iter);
}

static void
test_overlayaz_new_instance(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;

    helper_overlayaz_default_values(o);
}

static void
test_overlayaz_reset(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;

    overlayaz_reset(o);
    assert_null(overlayaz_get_pixbuf(o));
    assert_null(overlayaz_get_filename(o));
    helper_overlayaz_default_values(o);
    assert_int_equal(overlayaz_changed(o), FALSE);
}

static void
test_overlayaz_reset_after_changes(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;
    GdkRGBA color;

    gdk_rgba_parse(&color, "rgba(0x12,0x34,0x56,0.5)");

    overlayaz_set_filename(o, "test");
    overlayaz_set_pixbuf(o, gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 111, 222));
    overlayaz_set_rotation(o, 11.1);
    overlayaz_set_location(o, &(struct overlayaz_location){56.0, 23.0, 101.1});
    overlayaz_set_ref_one(o, OVERLAYAZ_REF_AZ, &(struct overlayaz_location){56.05, 23.05, 100.0}, 55.0, 100.0);
    overlayaz_set_ref_one(o, OVERLAYAZ_REF_EL, &(struct overlayaz_location){56.05, 23.05, 100.0}, 66.0, 100.0);
    overlayaz_set_grid(o, OVERLAYAZ_REF_AZ, TRUE);
    overlayaz_set_grid(o, OVERLAYAZ_REF_EL, FALSE);
    overlayaz_set_grid_step(o, OVERLAYAZ_REF_AZ, 0.5);
    overlayaz_set_grid_step(o, OVERLAYAZ_REF_EL, 0.25);
    overlayaz_set_grid_position(o, OVERLAYAZ_REF_AZ, 12.3);
    overlayaz_set_grid_position(o, OVERLAYAZ_REF_EL, 45.6);
    overlayaz_set_grid_width(o, 32.0);
    overlayaz_set_grid_color(o, &color);
    overlayaz_set_grid_font(o, "monospace 64");
    overlayaz_set_grid_font_color(o, &color);
    overlayaz_marker_list_add(overlayaz_get_marker_list(o), overlayaz_marker_new());

    overlayaz_reset(o);
    helper_overlayaz_default_values(o);
    assert_int_equal(overlayaz_changed(o), FALSE);
}

static void
test_overlayaz_changed(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;

    overlayaz_unchanged(o);
    assert_false(overlayaz_changed(o));
    overlayaz_set_filename(o, "test");
    assert_true(overlayaz_changed(o));
    overlayaz_unchanged(o);
    overlayaz_set_filename(o, "test2");
    assert_true(overlayaz_changed(o));
}

static void
test_overlayaz_filename_change(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;
    const gchar filename[] = "/tmp/test";

    assert_int_equal(overlayaz_changed(o), FALSE);
    overlayaz_set_filename(o, filename);
    assert_int_equal(overlayaz_changed(o), TRUE);
    assert_string_equal(overlayaz_get_filename(o), filename);

    overlayaz_unchanged(o);
    overlayaz_set_filename(o, filename);
    assert_int_equal(overlayaz_changed(o), FALSE);
}

static void
test_overlayaz_pixbuf_change(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;
    gint width = 123;
    gint height = 345;
    GdkPixbuf *pixbuf;

    assert_int_equal(overlayaz_changed(o), FALSE);
    pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, width, height);
    assert_non_null(pixbuf);
    overlayaz_set_pixbuf(ctx->o, pixbuf);
    assert_int_equal(overlayaz_changed(o), TRUE);
    assert_ptr_equal(overlayaz_get_pixbuf(o), pixbuf);
    assert_int_equal(overlayaz_get_width(o), width);
    assert_int_equal(overlayaz_get_height(o), height);
}

static void
test_overlayaz_rotation_change(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;
    gdouble angle = 45.0;

    assert_int_equal(overlayaz_changed(o), FALSE);
    overlayaz_set_rotation(ctx->o, angle);
    assert_int_equal(overlayaz_changed(o), TRUE);
    assert_float_equal(overlayaz_get_rotation(o), angle, FLT_EPSILON);

    overlayaz_unchanged(o);
    overlayaz_set_rotation(ctx->o, angle);
    assert_int_equal(overlayaz_changed(o), FALSE);
}

static void
test_overlayaz_rotation_change_overlap(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;
    gdouble angle = 365.0;

    assert_int_equal(overlayaz_changed(o), FALSE);
    overlayaz_set_rotation(ctx->o, 365.0);
    assert_int_equal(overlayaz_changed(o), TRUE);
    assert_float_equal(overlayaz_get_rotation(o), angle-360.0, FLT_EPSILON);

    overlayaz_unchanged(o);
    overlayaz_set_rotation(ctx->o, angle-360.0);
    assert_int_equal(overlayaz_changed(o), FALSE);
}

static void
test_overlayaz_rotation_change_overlap_negative(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;
    gdouble angle = -375.0;

    assert_int_equal(overlayaz_changed(o), FALSE);
    overlayaz_set_rotation(ctx->o, angle);
    assert_int_equal(overlayaz_changed(o), TRUE);
    assert_float_equal(overlayaz_get_rotation(o), angle+360.0, FLT_EPSILON);

    overlayaz_unchanged(o);
    overlayaz_set_rotation(ctx->o, angle+360.0);
    assert_int_equal(overlayaz_changed(o), FALSE);
}

static void
test_overlayaz_location_undefined(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;
    struct overlayaz_location location;

    assert_false(overlayaz_get_location(o, &location));
}

static void
test_overlayaz_location_change(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;
    struct overlayaz_location location;
    gdouble new_latitude = 50.123456;
    gdouble new_longitude = 20.654321;
    gdouble new_altitude = 123.456;

    overlayaz_unchanged(o);
    overlayaz_set_location(o, &(struct overlayaz_location){new_latitude, new_longitude, new_altitude});
    assert_true(overlayaz_changed(o));
    assert_true(overlayaz_get_location(o, &location));
    assert_float_equal(new_latitude, location.latitude, FLT_EPSILON);
    assert_float_equal(new_longitude, location.longitude, FLT_EPSILON);
    assert_float_equal(new_altitude, location.altitude, FLT_EPSILON);

    overlayaz_unchanged(o);
    overlayaz_set_location(o, &location);
    assert_int_equal(overlayaz_changed(o), FALSE);
}

static void
helper_overlayaz_undefined_ref(const overlayaz_t       *o,
                               enum overlayaz_ref_type  type,
                               enum overlayaz_ref_id    id)
{
    struct overlayaz_location location;
    gdouble position;

    assert_false(overlayaz_get_ref_location(o, type, id, &location));
    assert_false(overlayaz_get_ref_position(o, type, id, &position));
}

static void
test_overlayaz_refs_undefined(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;

    helper_overlayaz_undefined_ref(o, OVERLAYAZ_REF_AZ, OVERLAYAZ_REF_A);
    helper_overlayaz_undefined_ref(o, OVERLAYAZ_REF_AZ, OVERLAYAZ_REF_B);
    helper_overlayaz_undefined_ref(o, OVERLAYAZ_REF_EL, OVERLAYAZ_REF_A);
    helper_overlayaz_undefined_ref(o, OVERLAYAZ_REF_EL, OVERLAYAZ_REF_B);
}

static void
test_overlayaz_ratio_undefined(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;
    gdouble ratio, ratio2;

    assert_false(overlayaz_get_ratio(o, OVERLAYAZ_REF_AZ, &ratio));
    assert_false(overlayaz_get_ratio(o, OVERLAYAZ_REF_EL, &ratio2));
}

static void
helper_overlayaz_grid_change(overlayaz_t             *o,
                             enum overlayaz_ref_type  type)
{
    gboolean grid_state;
    grid_state = overlayaz_get_grid(o, type);
    overlayaz_unchanged(o);
    overlayaz_set_grid(o, type, !grid_state);
    assert_int_equal(overlayaz_get_grid(o, type), !grid_state);
    assert_true(overlayaz_changed(o));
    overlayaz_unchanged(o);
    overlayaz_set_grid(o, type, !!grid_state);
    assert_int_equal(overlayaz_get_grid(o, type), !!grid_state);
    assert_true(overlayaz_changed(o));
}

static void
test_overlayaz_grid_change(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;

    helper_overlayaz_grid_change(o, OVERLAYAZ_REF_AZ);
    helper_overlayaz_grid_change(o, OVERLAYAZ_REF_EL);
}

static void
test_overlayaz_grid_change_crosscheck(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;

    overlayaz_set_grid(o, OVERLAYAZ_REF_AZ, FALSE);
    overlayaz_set_grid(o, OVERLAYAZ_REF_EL, TRUE);
    assert_int_equal(overlayaz_get_grid(o, OVERLAYAZ_REF_AZ), FALSE);
    overlayaz_set_grid(o, OVERLAYAZ_REF_AZ, FALSE);
    assert_int_equal(overlayaz_get_grid(o, OVERLAYAZ_REF_EL), TRUE);
}


static void
helper_overlayaz_ref_one(overlayaz_t               *o,
                         enum overlayaz_ref_type    type,
                         struct overlayaz_location  locationA,
                         gdouble                    positionA,
                         gdouble                    ratio)
{
    struct overlayaz_location location;
    gdouble position, tmp;

    overlayaz_unchanged(o);
    overlayaz_set_ref_one(o, type,
                          &locationA,
                          positionA, ratio);
    assert_true(overlayaz_changed(o));

    assert_true(overlayaz_get_ref_location(o, type, OVERLAYAZ_REF_A, &location));
    assert_true(overlayaz_get_ref_position(o, type, OVERLAYAZ_REF_A, &position));
    assert_float_equal(locationA.latitude, location.latitude, FLT_EPSILON);
    assert_float_equal(locationA.longitude, location.longitude, FLT_EPSILON);
    assert_float_equal(locationA.altitude, location.altitude, FLT_EPSILON);
    assert_float_equal(positionA, position, FLT_EPSILON);
    assert_false(overlayaz_get_ref_location(o, type, OVERLAYAZ_REF_B, &location));
    assert_false(overlayaz_get_ref_position(o, type, OVERLAYAZ_REF_B, &position));
    assert_true(overlayaz_get_ratio(o, type, &tmp));
    assert_float_equal(ratio, tmp, FLT_EPSILON);
}

static void
helper_overlayaz_ref_two(overlayaz_t              *o,
                         enum overlayaz_ref_type   type,
                         struct overlayaz_location locationA,
                         gdouble                   positionA,
                         struct overlayaz_location locationB,
                         gdouble                   positionB,
                         gdouble                   ratio_expected)
{
    struct overlayaz_location location;
    gdouble position, ratio;

    overlayaz_unchanged(o);
    overlayaz_set_ref_two(o, type,
                          &locationA, positionA,
                          & locationB, positionB);
    assert_true(overlayaz_changed(o));

    assert_true(overlayaz_get_ref_location(o, type, OVERLAYAZ_REF_A, &location));
    assert_true(overlayaz_get_ref_position(o, type, OVERLAYAZ_REF_A, &position));
    assert_float_equal(locationA.latitude, location.latitude, FLT_EPSILON);
    assert_float_equal(locationA.longitude, location.longitude, FLT_EPSILON);
    assert_float_equal(locationA.altitude, location.altitude, FLT_EPSILON);
    assert_float_equal(positionA, position, FLT_EPSILON);

    assert_true(overlayaz_get_ref_location(o, type, OVERLAYAZ_REF_B, &location));
    assert_true(overlayaz_get_ref_position(o, type, OVERLAYAZ_REF_B, &position));
    assert_float_equal(locationB.latitude, location.latitude, FLT_EPSILON);
    assert_float_equal(locationB.longitude, location.longitude, FLT_EPSILON);
    assert_float_equal(locationB.altitude, location.altitude, FLT_EPSILON);
    assert_float_equal(positionB, position, FLT_EPSILON);

    assert_true(overlayaz_get_ratio(o, type, &ratio));
    assert_float_equal(ratio_expected, ratio, 1e-04);
}

static void
test_overlayaz_ref_one_azimuth(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;
    gdouble latitude = 50.05;
    gdouble longitude = 20.05;
    gdouble altitude = 0.0;
    gdouble position = 250.0;
    gdouble ratio = 100.0;

    helper_overlayaz_ref_one(o, OVERLAYAZ_REF_AZ,
                            (struct overlayaz_location){latitude, longitude, altitude},
                            position, ratio);
}

static void
test_overlayaz_ref_two_azimuth(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;
    gdouble latitudeA = 50.05;
    gdouble longitudeA = 20.05;
    gdouble altitudeA = 222.2;
    gdouble positionA = 250.0;
    gdouble latitudeB = 51.21994473;
    gdouble longitudeB = 21.51716805;
    gdouble altitudeB = 333.3;
    gdouble positionB = 750.0;
    gdouble ratio_expected = 100.0;

    helper_overlayaz_ref_two(o, OVERLAYAZ_REF_AZ,
                             (struct overlayaz_location){latitudeA, longitudeA, altitudeA}, positionA,
                             (struct overlayaz_location){latitudeB, longitudeB, altitudeB}, positionB,
                             ratio_expected);

    helper_overlayaz_ref_two(o, OVERLAYAZ_REF_AZ,
                             (struct overlayaz_location){latitudeA, longitudeA, altitudeA}, positionA,
                             (struct overlayaz_location){latitudeB, longitudeB, altitudeB}, positionB,
                              ratio_expected);
}


static void
test_overlayaz_ref_one_elevation(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;
    gdouble latitude = 50.05;
    gdouble longitude = 20.05;
    gdouble altitude = 100.0;
    gdouble position = 250.0;
    gdouble ratio = 100.0;

    helper_overlayaz_ref_one(o, OVERLAYAZ_REF_EL,
                             (struct overlayaz_location){latitude, longitude, altitude},
                             position, ratio);
}

static void
test_overlayaz_ref_two_elevation(void **state)
{
    test_context_t *ctx = *state;
    overlayaz_t *o = ctx->o;
    gdouble latitudeA = 50.05;
    gdouble longitudeA = 20.05;
    gdouble altitudeA = 100.0;
    gdouble positionA = 500.0;
    gdouble latitudeB = 50.05;
    gdouble longitudeB = 20.06;
    gdouble altitudeB = 200.0;
    gdouble positionB = 423.81835;
    gdouble ratio_expected = 100.0;

    helper_overlayaz_ref_two(o, OVERLAYAZ_REF_EL,
                             (struct overlayaz_location){latitudeA, longitudeA, altitudeA}, positionA,
                             (struct overlayaz_location){latitudeB, longitudeB, altitudeB}, positionB,
                             ratio_expected);

    helper_overlayaz_ref_two(o, OVERLAYAZ_REF_EL,
                             (struct overlayaz_location){latitudeB, longitudeB, altitudeB}, positionB,
                             (struct overlayaz_location){latitudeA, longitudeA, altitudeA}, positionA,
                             ratio_expected);
}

const struct CMUnitTest tests[] =
{
    /* Setup with no configuration */
    cmocka_unit_test_setup_teardown(test_overlayaz_new_instance, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_reset, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_reset_after_changes, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_changed, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_filename_change, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_pixbuf_change, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_rotation_change, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_rotation_change_overlap, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_rotation_change_overlap_negative, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_location_undefined, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_location_change, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_refs_undefined, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_ratio_undefined, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_grid_change, test_setup, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_grid_change_crosscheck, test_setup, test_teardown),
    /* Setup with location */
    cmocka_unit_test_setup_teardown(test_overlayaz_ref_one_azimuth, test_setup2, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_ref_two_azimuth, test_setup2, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_ref_one_elevation, test_setup2, test_teardown),
    cmocka_unit_test_setup_teardown(test_overlayaz_ref_two_elevation, test_setup2, test_teardown),
    /* Setup with location and references */
};

int
main(void)
{
    overlayaz_geo_init();
    return cmocka_run_group_tests(tests, group_setup, group_teardown);
}
