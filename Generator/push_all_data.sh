#! /bin/bash

mkdir gzip
for file in $1/*
do
	filename=$(basename $file)
	gzip -c $file > gzip/$filename
	aws s3api put-object \
		--bucket vtk-wasm-examples \
		--key data/$filename \
		--body gzip/$filename \
		--content-encoding gzip \
		--acl public-read
done
rm -r gzip
