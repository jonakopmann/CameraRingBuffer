set(RingBuffer_files RingBuffer.c RingBuffer.h)

add_library(RingBuffer ${RingBuffer_files})

find_package(PkgConfig)
find_program(GLIB2_MKENUMS glib-mkenums REQUIRED)
pkg_check_modules(GLIB2 glib-2.0>=2.38 REQUIRED)
pkg_check_modules(GOBJECT2 gobject-2.0>=2.38 REQUIRED)
pkg_check_modules(GMODULE2 gmodule-2.0>=2.38 REQUIRED)

link_directories(${GLIB_LIBRARY_DIRS})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLIB2_INCLUDE_DIRS}
    ${GOBJECT2_INCLUDE_DIRS}
    ${GMODULE2_INCLUDE_DIRS}
)

set(RINGBUFFER_DEPS 
    ${GLIB2_LIBRARIES}
    ${GOBJECT2_LIBRARIES}
    ${GMODULE2_LIBRARIES})

target_link_libraries(RingBuffer ${RINGBUFFER_DEPS})