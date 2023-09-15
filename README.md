# VTK Examples compilation in WASM

## Current state

Generator can be used to create CMakelists of examples which do not require to call arguments when ran (all the one that were already tested that require arguments have the flag "wrong usage" in doesntwork.txt).
doesntwork.txt contains all known examples that either can't build or crash, with details.
Directories in repo root are the one that are already tested.

## How to build examples

Works by topics: look for the topic you want from [VTK Examples][vtk-examples].

Then use Generator to create examples:

``` bash
cd Generator
./GenerateExamplesWASM.sh <path_to_vtk> sources/<topic_name> ../<topic_name>
```

It's now time to use [vtk-wasm docker image][docker] to build and try your example !

``` bash
cd ../<topic_name>
docker run --rm --entrypoint /bin/bash -v $PWD:/work -it kitware/vtk-wasm
mkdir build && cd build

cmake -GNinja \
  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE} \
  -DVTK_DIR=/VTK-install/Release/lib/cmake/vtk \
  -DCMAKE_BUILD_TYPE=Release \
  ../<example_name>

cmake --build .
exit
```

## Run

Now that your example was built, you only have to start a web server to get it from your browser

``` bash
cd build
python -m http.server 2000
```

Open browser at localhost:2000 and boom (well... If you have the chance to have built a working example)

[vtk-examples]: https://examples.vtk.org/
[docker]: https://hub.docker.com/r/kitware/vtk-wasm
