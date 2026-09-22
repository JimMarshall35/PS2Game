#!/usr/bin/env bash

#################
#     Tools     #
#################

texture_maker="tools/texture_maker/build/TextureMaker"
packer="tools/packer/build/Packer"

#################
#    Textures   #
#################

input_textures=(
    "./assets/source/Material.001 Base Color.png"
)

declare -A texture_output_paths=(
    ["./assets/source/Material.001 Base Color.png"]="./assets/compiled/paddle.texture"
)

declare -A texture_CLUT_output_path=(
    ["./assets/source/Material.001 Base Color.png"]="./assets/compiled/paddle.clut"
)

declare -A texture_output_formats=(
    ["./assets/source/Material.001 Base Color.png"]="" # blank - 8 bit per pixel default
)

declare -A texture_CLUT_output_formats=(
    ["./assets/source/Material.001 Base Color.png"]="" # blank - PSMCT32 default
)

#################
#   Pak files   #
#################

declare -A pak_files=(
    ["./assets/main_pak_manifest.json"]="./assets/compiled/main.pak"
)



#################
#      Main     #
#################
# Assumes script is in a ./scripts folder at the root of the repo, set repo root as cwd
pushd "$(dirname "$(readlink -f "$0")")/.."
    if [[ ! -f "$texture_maker" ]]; then
        echo "texture_maker missing - you need to build it"
        exit 1
    fi
    if [[ ! -f "$packer" ]]; then
        echo "packer missing - you need to build it"
        exit 1
    fi

    mkdir -p assets/compiled 
    
    for texture in "${input_textures[@]}"; do
        # lookup texture maker args
        output_path="${texture_output_paths[$texture]}"
        clut_output_path="${texture_CLUT_output_path[$texture]}"
        texture_output_format="${texture_output_formats[$texture]}"
        texture_CLUT_output_format="${texture_CLUT_output_formats[$texture]}"

        # invoke texture maker
        echo "Converting texture: $texture. Output path: $output_path CLUT output path: $clut_output_path"
        $texture_maker -p "$texture" -o "$output_path" -l "$clut_output_path" $texture_output_format $texture_CLUT_output_format
    done

    for manifest in "${!pak_files[@]}"; do
        pak_file_output="${pak_files[$manifest]}"
        echo "Making .pak file: $pak_file_output from manifest $manifest"
        $packer -m "$manifest" -o "$pak_file_output"
    done
popd