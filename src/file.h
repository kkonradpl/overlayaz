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

#ifndef OVERLAYAZ_FILE_H_
#define OVERLAYAZ_FILE_H_

enum overlayaz_file_load_error
{
    OVERLAYAZ_FILE_LOAD_OK,
    OVERLAYAZ_FILE_LOAD_ERROR_IMAGE_OPEN,
    OVERLAYAZ_FILE_LOAD_ERROR_IMAGE_TOO_BIG,
    OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_OPEN,
    OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_PARSE,
    OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_FORMAT,
    OVERLAYAZ_FILE_LOAD_ERROR_PROFILE_VERSION
};

enum overlayaz_file_load_error overlayaz_file_load(overlayaz_t*, const gchar*);
const gchar* overlayaz_file_load_error(enum overlayaz_file_load_error);

#endif
