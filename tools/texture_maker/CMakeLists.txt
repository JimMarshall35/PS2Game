cmake_minimum_required(VERSION 3.16)

project(TextureMaker C)

find_package(OpenMP)

add_subdirectory(./libimagequant-2.x)

add_executable(TextureMaker
    src/main.c
    src/IDTEX8_CSM1_CLUTSwizzleTable.c
)

target_include_directories(TextureMaker PRIVATE
    ../../include
    ./include
    ./libimagequant-2.x
)

target_link_libraries(TextureMaker PRIVATE imagequant_a m)

if(OpenMP_C_FOUND)
  target_link_libraries(TextureMaker PRIVATE OpenMP::OpenMP_C)
endif()