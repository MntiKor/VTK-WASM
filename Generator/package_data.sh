#!/bin/bash

emsdk_path=$1

for f in Data/*
do
    filename=$(basename ${f})
    ${emsdk_path}/upstream/emscripten/tools/file_packager packaged_data/${filename}.data --preload ${f}@/ --js-output=packaged_data/${filename}.js
done
