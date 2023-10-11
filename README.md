# VTK Examples compilation in WASM

It's a work in progress that you can see [there][MntiKor-examples].

## Current state

doesntwork.txt contains all known examples that either can't build or crash, with details.

## How to build examples

Works by topics: look for the Cxx topic you want from [VTK Examples][vtk-examples].

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

Open browser at localhost:2000 and boom (well... If you have the chance to have built a working example) !

[MntiKor-examples]: https://mntikor.github.io/vtk-examples
[vtk-examples]: https://examples.vtk.org/
[docker]: https://hub.docker.com/r/kitware/vtk-wasm
