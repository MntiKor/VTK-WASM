#!/bin/bash

if [ $# != 2 ]
then
    echo "Usage: ./push_example.sh <example_name> <build_dir>"
    exit 1
fi

example_name=$1
build_dir=$2

pushd ${build_dir}

gzip index.html
aws s3api put-object \
    --bucket vtk-wasm-examples \
    --key ${example_name}/index.html \
    --content-encoding gzip \
    --body index.html.gz \
    --acl public-read

gzip ${example_name}.js
aws s3api put-object \
    --bucket vtk-wasm-examples \
    --key ${example_name}/${example_name}.js \
    --body ${example_name}.js.gz \
    --content-encoding gzip \
    --acl public-read

gzip ${example_name}.wasm
aws s3api put-object \
    --bucket vtk-wasm-examples \
    --key ${example_name}/${example_name}.wasm \
    --body ${example_name}.wasm.gz \
    --content-type application/wasm \
    --content-encoding gzip \
    --acl public-read

popd
