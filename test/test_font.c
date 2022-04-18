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
#include "font.h"

static const char default_font[] = "monospace 12";

typedef struct {
    overlayaz_font_t *f;
} test_context_t;

static int
groupSetup(void **state)
{
    test_context_t *ctx = malloc(sizeof(test_context_t));
    *state = ctx;
    return 0;
}
static int
groupTeardown(void **state)
{
    test_context_t *ctx = *state;
    free(ctx);
    return 0;
}

static int
testSetup(void **state) {
    test_context_t *ctx = *state;
    ctx->f = overlayaz_font_new(default_font);
    return 0;
}

static int
testTeardown (void **state) {
    test_context_t *ctx = *state;
    overlayaz_font_free(ctx->f);
    return 0;
}

static void
font_test_new_instance(void **state)
{
    test_context_t *ctx = *state;
    PangoFontDescription *desc;

    assert_string_equal(overlayaz_font_get(ctx->f), default_font);
    assert_non_null(overlayaz_font_get_pango(ctx->f));
    desc = pango_font_description_from_string(default_font);
    assert_int_equal(pango_font_description_equal(overlayaz_font_get_pango(ctx->f), desc), TRUE);
    pango_font_description_free(desc);
}

static void
font_test_change_font(void **state)
{
    test_context_t *ctx = *state;
    const char new_font[] = "sans-serif 12";
    PangoFontDescription *desc;

    overlayaz_font_set(ctx->f, new_font);
    assert_string_equal(overlayaz_font_get(ctx->f), new_font);
    assert_non_null(overlayaz_font_get_pango(ctx->f));
    desc = pango_font_description_from_string(new_font);
    assert_int_equal(pango_font_description_equal(overlayaz_font_get_pango(ctx->f), desc), TRUE);
    pango_font_description_free(desc);
}

const struct CMUnitTest tests[] = {
    cmocka_unit_test_setup_teardown(font_test_new_instance, testSetup, testTeardown),
    cmocka_unit_test_setup_teardown(font_test_change_font, testSetup, testTeardown),
};

int
main(void)
{
    return cmocka_run_group_tests(tests, groupSetup, groupTeardown);
}
