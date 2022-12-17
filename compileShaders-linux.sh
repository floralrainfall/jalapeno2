#!/bin/bash

# linux shader compile

SHADERS=(textured simple snapshot bspmap)
SINGLESHADERS=("skinned;vertex;skinned" "textured;fragment;skinned" "simple;vertex;quick" "quick;fragment;quick")
PLATFORM=linux
PROFILE=NNN -p NNN_es
export SHADERC=./build/deps/bgfx.cmake/shaderc
cd build && ninja shaderc && cd ..

mkdir -p ./build/shader

log() {
    echo "===========>" $1
    echo "===========>" $1 "<===========" >> shaderlog.txt
}

compile_shader() {
    local TYPES="?"
    case $2 in 
        "vertex")
            TYPES="v"
            ;;
        "fragment")
            TYPES="f"
            ;;
    esac
    log "COMPILE:   $1  $2   (${TYPES}_$1.sc)"
    OUTPUT=""
    [ ! -z $3 ] && OUTPUT=${TYPES}_$3
    [ -z "$OUTPUT" ] && OUTPUT="${TYPES}_$1"
    log "output == ${OUTPUT}"
    $SHADERC -f "data/shader/${TYPES}_$1.sc" -o "build/shader/${OUTPUT}.bin" \
        --platform ${PLATFORM} --type $2 --verbose -i ./shader -p ${PROFILE} >> shaderlog.txt
    log "COMPILE:   returned    $?"
}

compile_program() {
    compile_shader $1 "vertex"
    compile_shader $1 "fragment"
}

rm shaderlog.txt
log "UTILITY: compiling normal shaders"
for shader in "${SHADERS[@]}"
do
    log "PROGRAM: compiling $shader"
    compile_program $shader
done
log "UTILITY: compiling custom shaders"
for shaderpair in "${SINGLESHADERS[@]}"
do
    log "PROGRAM: compiling (p) $shader"
    IFS=";" read -r -a pair <<< "${shaderpair}"
    compile_shader ${pair[0]} ${pair[1]} ${pair[2]}
done