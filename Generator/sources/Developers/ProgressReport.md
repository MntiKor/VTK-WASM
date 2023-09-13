### Description

This example demonstrates how to get the progress of a filter. This requires that the filter is updating its progress in a sensible way. A sample filter is provided which loops through the input points and updates its progress along the way.

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
