cmake_minimum_required(VERSION 3.10)

project(CameraRingbuffer
        VERSION 1.0.0)

set(control_files src/control.c src/RingBuffer.c)

add_executable(RingbufferControl ${control_files})

target_include_directories(RingbufferControl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(PkgConfig)
pkg_check_modules(LIBUCA libuca REQUIRED)
pkg_check_modules(GLIB2 glib-2.0>=2.38 REQUIRED)
pkg_check_modules(GOBJECT2 gobject-2.0>=2.38 REQUIRED)
pkg_check_modules(GMODULE2 gmodule-2.0>=2.38 REQUIRED)


link_directories(${GLIB2_LIBRARY_DIRS})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${GLIB2_INCLUDE_DIRS}
    ${GOBJECT2_INCLUDE_DIRS}
    ${GMODULE2_INCLUDE_DIRS}
)

set(DEPS 
    ${GLIB2_LIBRARIES}
    ${GOBJECT2_LIBRARIES}
    ${GMODULE2_LIBRARIES}
    ${LIBUCA_LIBRARIES}
    )

target_link_libraries(RingbufferControl PUBLIC ${DEPS} pthread)