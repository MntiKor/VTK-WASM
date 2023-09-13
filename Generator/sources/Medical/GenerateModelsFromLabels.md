### Description

This example uses vtkDiscreteFlyingEdges3D or vtkDiscreteMarchingCubes to create vtkPolyData models from a 3D volume that contains discrete labels. These volumes are normally the output of a segmentation algorithm. The polydata for each label will be output into a separate file.

You can load these files into ParaView, where they will appear as a series of time steps. You can then single step through displaying the polydate from each file making up the series.

If you want to see the segmentation results as cube models, see the example [GenerateCubesFromLabels](../GenerateCubesFromLabels)

The input volume must be in [MetaIO format](http://www.vtk.org/Wiki/MetaIO/Documentation).

Once you generate the models, you can view them with [Paraview](http://paraview.org)

!!! note
    This original source code for this example is [here](https://gitlab.kitware.com/vtk/vtk/blob/395857190c8453508d283958383bc38c9c2999bf/Examples/Medical/Cxx/GenerateModelsFromLabels.cxx).
