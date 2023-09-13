### Description

This example demonstrates how to create a filter that takes a vtkGraph as input and produces a vtkGraph as output.

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
