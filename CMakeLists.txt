cmake_minimum_required(VERSION 3.16)

project(PS2Game C)

set(PLATFORM "LINUX" CACHE STRING "Target platform (LINUX or PS2)")
set_property(CACHE PLATFORM PROPERTY STRINGS LINUX PS2)

add_subdirectory(src/external/cglm/ EXCLUDE_FROM_ALL)


set(PLATFORM_SPECIFIC_SOURCES)

if(PLATFORM STREQUAL "PS2")
    set(CMAKE_EXECUTABLE_SUFFIX ".elf")

    # postprocess the .vcl files
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/draw_3D.vsm
        COMMAND openvcl ${CMAKE_CURRENT_SOURCE_DIR}/src/ps2/vcl/draw_3D.vcl > ${CMAKE_CURRENT_BINARY_DIR}/draw_3D.vsm
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/ps2/vcl/draw_3D.vcl
    )

    # assemble vsm files to object files
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/draw_3D.o
        COMMAND dvp-as ${CMAKE_CURRENT_BINARY_DIR}/draw_3D.vsm -o ${CMAKE_CURRENT_BINARY_DIR}/draw_3D.o
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/draw_3D.vsm
    )

    # ps2 sdk includes
    target_include_directories(Game PRIVATE
        "$ENV{PS2SDK}/ee/include"
        "$ENV{PS2SDK}/ee/math3d/include"
        "$ENV{PS2SDK}/common/include"
    )

    # ps2 sdk libraries
    target_link_directories(Game PRIVATE
        "$ENV{PS2SDK}/ee/lib"
    )

    target_compile_definitions(Game PRIVATE
        _EE
    )

    target_compile_options(Game PRIVATE
        -G0
        -O2
        -Wall
    )

    target_link_libraries(Game PRIVATE
        kernel
        packet
        packet2
        dma
        graph
        draw
        math3d
    )

    list(APPEND PLATFORM_SPECIFIC_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/draw_3D.o)
    list(APPEND PLATFORM_SPECIFIC_SOURCES src/ps2/PS2Renderer.c)
    list(APPEND PLATFORM_SPECIFIC_SOURCES src/ps2/PS2Platform.c)

elseif(PLATFORM STREQUAL "LINUX")
    list(APPEND PLATFORM_SPECIFIC_SOURCES src/linux/OpenGLRenderer.c)
    list(APPEND PLATFORM_SPECIFIC_SOURCES src/linux/GLFWPlatform.c)

    find_package(glfw3 REQUIRED)
    target_link_libraries(Game PUBLIC glfw)

endif()

add_executable(Game
    src/pak.c
    src/main.c
    ${PLATFORM_SPECIFIC_SOURCES}
)

target_link_libraries(${LIBRARY_NAME} PRIVATE
  cglm_headers)