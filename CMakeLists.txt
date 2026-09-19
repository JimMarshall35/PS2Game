cmake_minimum_required(VERSION 3.16)

project(PS2Game C)

set(CMAKE_EXECUTABLE_SUFFIX ".elf")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/zbyszek.c
    COMMAND bin2c ${CMAKE_CURRENT_SOURCE_DIR}/zbyszek.raw ${CMAKE_CURRENT_SOURCE_DIR}/zbyszek.c zbyszek
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/zbyszek.raw
)

add_custom_command(
   OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/draw_3D.vsm
   COMMAND openvcl ${CMAKE_CURRENT_SOURCE_DIR}/draw_3D.vcl > ${CMAKE_CURRENT_SOURCE_DIR}/draw_3D.vsm
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/draw_3D.vcl
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/draw_3D.o
    COMMAND dvp-as ${CMAKE_CURRENT_SOURCE_DIR}/draw_3D.vsm -o ${CMAKE_CURRENT_BINARY_DIR}/draw_3D.o
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/draw_3D.vsm
)

add_executable(vu1
    main.c
    mesh_data.c
    zbyszek.c
    ${CMAKE_CURRENT_BINARY_DIR}/draw_3D.o
)

target_include_directories(vu1 PRIVATE
    "$ENV{PS2SDK}/ee/include"
    "$ENV{PS2SDK}/ee/math3d/include"
    "$ENV{PS2SDK}/common/include"
)

target_link_directories(vu1 PRIVATE
    "$ENV{PS2SDK}/ee/lib"
)

target_compile_definitions(vu1 PRIVATE
    _EE
)

target_compile_options(vu1 PRIVATE
    -G0
    -O2
    -Wall
)

target_link_libraries(vu1 PRIVATE
    kernel
    packet
    packet2
    dma
    graph
    draw
    math3d
)

