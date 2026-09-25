#!/usr/bin/env bash

#################
#     Tools     #
#################

texture_maker="tools/texture_maker/build/TextureMaker"
packer="tools/packer/build/Packer"
blender_exporter="tools/blender_exporter/exporter_cli.py"

##################
# Blender Scenes #
##################

declare -A blender_scenes=(
    # path to scene                         path to pak file manifestit goes into
    ["./assets/source/test_scene.blend"]="./assets/main_pak_manifest.json"
)


##################
#    Pak files   #
##################

declare -A pak_files=(
    # path to pak file manifest            output .pak file path
    ["./assets/main_pak_manifest.json"]="./assets/compiled/main.pak"
)

##################
#      Main      #
##################
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
    
    for scene in "${!blender_scenes[@]}"; do
        manifest="${blender_scenes[$scene]}"
        blender --background "$scene" --python $blender_exporter -- \
            --pak_manifest $manifest \
            --output_directory ./assets/compiled \
            --platform "PC" # hard code for now
    done


    # for manifest in "${!pak_files[@]}"; do
    #     pak_file_output="${pak_files[$manifest]}"
    #     echo "Making .pak file: $pak_file_output from manifest $manifest"
    #     $packer -m "$manifest" -o "$pak_file_output"
    # done
popd