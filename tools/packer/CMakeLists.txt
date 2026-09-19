cmake_minimum_required(VERSION 3.16)

project(Packer C)

add_executable(Packer
    src/main.c
    src/DynArray.c
    cJSON-1.7.19/cJSON.c
    cJSON-1.7.19/cJSON_Utils.c
)

target_include_directories(Packer PRIVATE
    ./cJSON-1.7.19
    ../../include
    ./include
)
