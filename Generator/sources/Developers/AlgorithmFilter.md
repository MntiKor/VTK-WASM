### Description

This example demonstrates how to create a filter that accepts a custom class and returns a custom class. To test that it is working, the class vtkTest simply stores a double named 'Value' that is instantiated to the value of 4.5. Example.cxx instantiates a vtkTest and sets its value to 5.6. The filter adds 1.0 to this value, so the vtkTest that is the output of the vtkTestAlgorithmFilter should contain the value 6.6.

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
