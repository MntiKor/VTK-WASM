### Description

Demonstrate a cone using the vtkDiscretizableColorTransferFunction to generate the colormap.

These two Python programs can be used to generate the function `def get_ctf(): ...` or `vtkNew<vtkDiscretizableColorTransferFunction> getCTF() ...` for either an XML description of a colormap or a JSON one.

- [ColorMapToLUT_XML](../../../Python/Utilities/ColorMapToLUT_XML/)
- [ColorMapToLUT_JSON](../../../Python/Utilities/ColorMapToLUT_JSON/)

Feel free to use either of these programs to generate different colormaps until you find one you like.

A good initial source for color maps is: [SciVisColor](https://sciviscolor.org/) -- this will provide you with plenty of XML examples.
