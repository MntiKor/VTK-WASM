# The "View Live" option (only available for Cxx examples)

## WebAssembly

WebAssembly (WASM) is a new way of running code inside a web page.
It's a technology which allows a C/C++ code to be executed
by the web browser as if it was a JS script.

## VTK-WASM

With the new web graphics APIs that appeared recently (WebGL, WebGPU...),
we have been able to build VTK for WebAssembly. Not all modules are available
but we keep on improving VTK-WASM, and this website is a display of what is
working, and what is not.

A docker image is available [here](https://hub.docker.com/r/kitware/vtk-wasm).

To build VTK-WASM manually, we use [Emscripten](https://emscripten.org/). You will
find instructions on how to build and use it [here](https://docs.vtk.org/en/latest/advanced/build_wasm_emscripten.html).

## Excluded Examples

Some examples do not implement a live preview feature in WebAssembly.
There are the reasons:

### No render

A lot of examples are here to show pipelines, algorithms, or utils that
does not always have a render (or whose render is not important), so we
chose not to display a live preview of those that would only show you text.

### Incompatible dependencies

Some examples use external dependencies that either could not be built in WebAssembly
(Qt) or are not Open Source (RenderMan).

Some VTK modules aren't WASM-compatible or not fully working,
like RenderingContextOpenGL2 or RenderingGL2PSOpenGL2.
Those can break some examples that are then removed from vtk-examples-wasm:

- Images/CombineImages

- Plotting/ChartMatrix, ChartOn3DScene, Diagram, MultiplePlots, ScatterPlot, SurfacePlot

### Irrelevant features

A few examples use features that aren't relevant with WebAssembly usage
(FullScreen, OpenVR).

There are examples which are just here to show different arguments
for the same code (Rotations{A,B,C,D}, WalkCow{A,B}...).
Those are also removed for practical reasons.

### Technical considerations

In order to improve build times, stability and bandwidth,
vtk-examples-wasm does not use Boost Library features even though it is provided
by Emscripten Ports. This forces us to remove a few examples:

- Graphs/AdjacentVertexIterator, BoostBreadthFirstSearchTree

- InfoVis/MutableGraphHelper

Second issue: VTK is initially writen for OpenGL2. As VTK-WASM uses
WebGL2 (OpenGL ES3), a lot of features need to be rewritten to comply
with WebGL standards. It takes a lot of time, and a few examples will have
to wait to be fully integrated:

- Shaders/BozoShader, BozoShaderDemo, CubeMap, MarbleShader, MarbleShaderDemo, SpatterShader

- Visualization/CorrectlyRenderTranslucentGeometry

- VolumeRendering/RayCastIsosurface

At last, the fact that WebAssembly runs within the browser implies that
resources are limited for the programs. We cannot give too much memory or workload
to a single-threaded program running inside a browser tab.

- Meshes/SubdivisionDemo requires ~1GB of memory to run a sufficient number of passes

- Visualization/FroggieSurface needs to allocate a single array of 2GB.
