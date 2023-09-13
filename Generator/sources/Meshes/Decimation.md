### Description

This example decimates a mesh using progressive decimation. The SetTargetReduction function specifies how many triangles should reduced by specifying the percentage (0,1) of triangles to be removed. For example, if the mesh contains 100 triangles and SetTargetReduction(.90) is called, after the decimation there will be approximately 10 triangles - a 90% reduction.

The implementation of vtkDecimatePro is similar to the algorithm originally described in ["Decimation of Triangle Meshes"](https://www.researchgate.net/publication/225075888_Decimation_of_triangle_meshes), Proc Siggraph \`92, with three major differences. First, this algorithm does not necessarily preserve the topology of the mesh. Second, it is guaranteed to give the a mesh reduction factor specified by the user (as long as certain constraints are not set - see Caveats). Third, it is set up generate progressive meshes, that is a stream of operations that can be easily transmitted and incrementally updated (see Hugues Hoppe's Siggraph '96 paper on [progressive meshes](http://hhoppe.com/pm.pdf)).

!!! seealso
    [QuadricClustering](../QuadricClustering) and [QuadricDecimation](../QuadricDecimation).
