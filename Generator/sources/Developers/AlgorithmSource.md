### Description

This example demonstrates how to create a source that returns a custom class. To test that it is working, the class vtkTest1 simply stores a double named 'Value' that is instantiated to the value of 4.5. Example.cxx instantiates a vtkTestSource which produces a vtkTest1.

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
