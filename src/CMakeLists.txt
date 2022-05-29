cmake_minimum_required(VERSION 3.6)

set(SOURCE_FILES
        geodesic/geodesic.c
        geodesic/geodesic.h
        conf.c
        conf.h
        dialog.c
        dialog.h
        dialog-about.c
        dialog-about.h
        dialog-alt.c
        dialog-alt.h
        dialog-export.c
        dialog-export.h
        dialog-info.c
        dialog-info.h
        draw.c
        draw.h
        exif.c
        exif.h
        export.c
        export.h
        file.c
        file.h
        font.c
        font.h
        geo.c
        geo.h
        grid-helper.c
        grid-helper.h
        icon.c
        icon.h
        location.h
        marker.c
        marker.h
        marker-iter.c
        marker-iter.h
        marker-list.c
        marker-list.h
        menu-grid.c
        menu-grid.h
        menu-help.c
        menu-help.h
        menu-marker.c
        menu-marker.h
        menu-ref.c
        menu-ref.h
        overlayaz.c
        overlayaz.h
        overlayaz-default.h
        profile.c
        profile.h
        srtm.c
        srtm.h
        ui.c
        ui.h
        ui-menu-grid.c
        ui-menu-grid.h
        ui-menu-marker.c
        ui-menu-marker.h
        ui-menu-ref.c
        ui-menu-ref.h
        ui-preferences.c
        ui-preferences.h
        ui-util.c
        ui-util.h
        ui-view-img.c
        ui-view-img.h
        ui-view-map.c
        ui-view-map.h
        window.c
        window.h)

set(SOURCE_FILES_MINGW
        mingw.c
        mingw.h)

set(ICON_MINGW
        icon.rc)

if(MINGW)
    IF(NOT (CMAKE_BUILD_TYPE MATCHES Debug))
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mwindows")
    ENDIF()
    set(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -i <SOURCE> -o <OBJECT>")

    add_library(liboverlayaz STATIC ${SOURCE_FILES} ${SOURCE_FILES_MINGW})
    add_executable(overlayaz main.c ${ICON_MINGW})
ELSE()
    add_library(liboverlayaz STATIC ${SOURCE_FILES})
    add_executable(overlayaz main.c)
ENDIF()

target_link_libraries(overlayaz liboverlayaz ${LIBRARIES})
