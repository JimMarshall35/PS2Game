cmake_minimum_required(VERSION 3.16)

project(PS2Game C)

set(PLATFORM "LINUX" CACHE STRING "Target platform (LINUX or PS2)")
set_property(CACHE PLATFORM PROPERTY STRINGS LINUX PS2)



set(PLATFORM_SPECIFIC_SOURCES)
set(PLATFORM_SPECIFIC_INCLUDES)
set(PLATFORM_SPECIFIC_LIBS)
set(PLATFORM_SPECIFIC_LINK_DIRECTORIES)
set(PLATFORM_SPECIFIC_COMPILE_DEFS)

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
    list(APPEND PLATFORM_SPECIFIC_INCLUDES
        "$ENV{PS2SDK}/ee/include"
        "$ENV{PS2SDK}/ee/math3d/include"
        "$ENV{PS2SDK}/common/include"
    )

    # ps2 sdk libraries
    list(APPEND PLATFORM_SPECIFIC_LINK_DIRECTORIES
        "$ENV{PS2SDK}/ee/lib"
    )

    list(APPEND PLATFORM_SPECIFIC_COMPILE_DEFS
        _EE
    )

    # target_compile_options(Game PRIVATE
    #     -G0
    #     -O2
    #     -Wall
    # )

    list(APPEND PLATFORM_SPECIFIC_LIBS
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
    list(APPEND PLATFORM_SPECIFIC_SOURCES src/vendor/glad/src/glad.c)
    list(APPEND PLATFORM_SPECIFIC_INCLUDES src/vendor/glad/include)
    list(APPEND PLATFORM_SPECIFIC_LIBS cglm_headers)

    find_package(glfw3 REQUIRED)
    list(APPEND PLATFORM_SPECIFIC_LIBS glfw)
    add_subdirectory(src/vendor/cglm-0.9.6/ EXCLUDE_FROM_ALL)

endif()

add_executable(Game
    src/engine/pak.c
    src/engine/Entity.c
    src/engine/FixedSizeObjectPool.c
    src/engine/Log.c
    src/engine/Memory.c
    src/engine/StackAllocator.c
    src/engine/Transform.c

    src/main.c

    ${PLATFORM_SPECIFIC_SOURCES}
)

target_include_directories(Game PRIVATE
    include
    ${PLATFORM_SPECIFIC_INCLUDES}
)


target_link_libraries(Game PRIVATE
  m
  ${PLATFORM_SPECIFIC_LIBS})

target_link_directories(Game PRIVATE
   ${PLATFORM_SPECIFIC_LINK_DIRECTORIES}
)

target_compile_definitions(Game PRIVATE
    ${PLATFORM_SPECIFIC_COMPILE_DEFS}
)
