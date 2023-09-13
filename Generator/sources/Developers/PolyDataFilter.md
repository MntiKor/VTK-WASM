### Description

This example demonstrates a filter named vtkTestPolyDataFilter that takes a vtkPolyData as input and produces a vtkPolyData as output.

You will need the following in your CMakeLists.txt file:

```cmake
find_package(VTK
 COMPONENTS
    CommonCore
    CommonDataModel
    CommonExecutionModel
    FiltersSources
    InfovisCore
)
```
