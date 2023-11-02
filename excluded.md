# Excluded Examples:

Some examples do not implement a live preview feature in WebAssembly.
There are the reasons:

## No render

A lot of examples are here do show pipelines, algorithms, or utils that
does not always have a render (or whose render is not important), so we
chose not to display a live preview of those that would only show you text.

## Uncompatible dependancies

Some examples use external dependancies that either could not be built in WebAssembly
(Qt) or are not Open Source (RenderMan).

Some VTK modules aren't WASM-compatible either,
like RenderingContextOpenGL2 or RenderingGL2PSOpenGL2.
Those can break some examples that are then removed from vtk-examples-wasm:

- DataStructures/KDTreeTimingDemo, ModifiedBSPTreeTimingDemo,
  OBBTreeTimingDemo, OctreeTimingDemo

- IO/ReadLegacyUnstructuredGrid

- Images/CombineImages

- Plotting/AreaPlot, BarChart, BoxChart, ChartMatrix, FunctionalBagPlot,
  Histogram2D, LinePlot, ParallelCoordinates, PieChart, PlotLine3D, ScatterPlot,
StackedBar, StackedPlot, SurfacePlot

## Irrelevant features

A few examples use features that aren't relevant with WebAssembly usage
(FullScreen, OpenVR).

There are examples which are just here to show different arguments
for the same code (Rotations{A,B,C,D}, WalkCow{A,B}...).
Those are also removed for practical reasons.

## Technical considerations

In order to increase build times, stability and bandwidth,
vtk-examples-wasm does not use Boost Library features even though it is provided
by Emscripten Ports. This forces us to remove a few examples:

- Graphs/AdjacentVertexIterator, BoostBreadthFirstSearchTree

- InfoVis/MutableGraphHelper

Second issue: vtk is initially writen for OpenGL2. As WebAssembly uses
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
