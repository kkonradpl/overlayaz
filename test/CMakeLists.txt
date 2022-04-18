cmake_minimum_required(VERSION 3.6)

find_package(CMOCKA REQUIRED)
include_directories(${CMOCKA_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(test_overlayaz test_overlayaz.c)
add_dependencies(test_overlayaz test_overlayaz liboverlayaz)
add_test(test_overlayaz test_overlayaz)
add_test(test_overlayaz_valgrind valgrind
         --error-exitcode=1 --read-var-info=yes
         --leak-check=full
         ./test_overlayaz)
target_link_libraries(test_overlayaz liboverlayaz cmocka ${LIBRARIES})

add_executable(test_font test_font.c)
add_dependencies(test_font test_font liboverlayaz)
add_test(test_font test_font)
add_test(test_font_valgrind valgrind
        --error-exitcode=1 --read-var-info=yes
        --leak-check=full
        ./test_font)

target_link_libraries(test_font liboverlayaz cmocka ${LIBRARIES})
