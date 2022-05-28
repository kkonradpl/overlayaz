cmake_minimum_required(VERSION 3.6)
project(overlayaz)

find_package(PkgConfig REQUIRED)

pkg_check_modules(GTK REQUIRED gtk+-3.0)
include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})
add_definitions(${GTK_CFLAGS_OTHER})

pkg_check_modules(SQLITE REQUIRED sqlite3)
include_directories(${SQLITE_INCLUDE_DIRS})
link_directories(${SQLITE_LIBRARY_DIRS})
add_definitions(${SQLITE_CFLAGS_OTHER})

pkg_check_modules(JSON-C REQUIRED json-c)
include_directories(${JSON-C_INCLUDE_DIRS})
link_directories(${JSON-C_LIBRARY_DIRS})
add_definitions(${JSON-C_CFLAGS_OTHER})

pkg_check_modules(OSMGPSMAP REQUIRED osmgpsmap-1.0)
include_directories(${OSMGPSMAP_INCLUDE_DIRS})
link_directories(${OSMGPSMAP_LIBRARY_DIRS})
add_definitions(${OSMGPSMAP_CFLAGS_OTHER})

pkg_check_modules(GEXIV2 REQUIRED gexiv2)
include_directories(${GEXIV2_INCLUDE_DIRS})
link_directories(${GEXIV2_LIBRARY_DIRS})
add_definitions(${GEXIV2_CFLAGS_OTHER})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -pedantic -std=c11")
enable_testing()

set(LIBRARIES
        ${GTK_LIBRARIES}
        ${SQLITE_LIBRARIES}
        ${JSON-C_LIBRARIES}
        ${OSMGPSMAP_LIBRARIES}
        ${GEXIV2_LIBRARIES}
        m)

add_subdirectory(src)
add_subdirectory(test)

if(NOT MINGW)
    install(TARGETS overlayaz DESTINATION bin)
    install(FILES overlayaz.desktop DESTINATION share/applications)
    install(DIRECTORY icons/ DESTINATION share/icons/hicolor
            PATTERN "INFO" EXCLUDE
            PATTERN "overlayaz.ico" EXCLUDE)
    install(CODE "execute_process(COMMAND gtk-update-icon-cache /usr/share/icons/hicolor)")
endif()
