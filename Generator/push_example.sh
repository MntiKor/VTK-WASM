#!/bin/bash

if [ $# != 2 ]
then
    echo "Usage: ./push_example.sh <example_name> <build_dir>"
    exit 1
fi

example_name=$1
build_dir=$2

pushd ${build_dir}

aws s3api put-object \
    --bucket vtk-wasm-examples \
    --key ${example_name}/index.html \
    --body index.html \
    --acl public-read

aws s3api put-object \
    --bucket vtk-wasm-examples \
    --key ${example_name}/${example_name}.js \
    --body ${example_name}.js \
    --acl public-read

aws s3api put-object \
    --bucket vtk-wasm-examples \
    --key ${example_name}/${example_name}.wasm \
    --body ${example_name}.wasm \
    --content-type application/wasm
    --acl public-read

popd
