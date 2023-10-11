#! /bin/bash

for file in $1/*
do
	filename=$(basename $file)
	aws s3api put-object \
		--bucket vtk-wasm-examples \
		--key data/$filename \
		--body $file \
		--acl public-read
done
