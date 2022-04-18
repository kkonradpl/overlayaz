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

#ifndef OVERLAYAZ_PROFILE_H_
#define OVERLAYAZ_PROFILE_H_

enum overlayaz_profile_load_error
{
    OVERLAYAZ_PROFILE_LOAD_OK,
    OVERLAYAZ_PROFILE_LOAD_ERROR_OPEN,
    OVERLAYAZ_PROFILE_LOAD_ERROR_PARSE,
    OVERLAYAZ_PROFILE_LOAD_ERROR_FORMAT,
    OVERLAYAZ_PROFILE_LOAD_ERROR_VERSION
};

enum overlayaz_profile_save_error
{
    OVERLAYAZ_PROFILE_SAVE_OK,
    OVERLAYAZ_PROFILE_SAVE_ERROR_OPEN,
    OVERLAYAZ_PROFILE_SAVE_ERROR_WRITE
};

enum overlayaz_profile_load_error overlayaz_profile_load(overlayaz_t*, const gchar*);
enum overlayaz_profile_save_error overlayaz_profile_save(const overlayaz_t*, const gchar*);

#endif
