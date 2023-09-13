### Description

Creates surfaces from labeled data. Volume data does not always contain samples of continuous data. A volume may contain discrete integer values, often the result of segmentation.
vtkDiscreteFlyingEdges3D or vtkDiscreteMarchingCubes create surfaces from these segmented volumes using a modified flying edges or marching cubes algorithm.
The algorithm generates one or more models representing the boundaries between the specified label and the adjacent structures. One or more label values must be specified to generate the models. The boundary positions are always defined to be half-way between adjacent voxels.

!!! seealso
    [SmoothDiscreteMarchingCubes](../SmoothDiscreteMarchingCubes) produces smooth models.
