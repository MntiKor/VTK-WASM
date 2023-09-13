### Description

This example shows how to produce a HTML page called [VTKNamedColorPatches](https://htmlpreview.github.io/?https://github.com/Kitware/vtk-examples/blob/gh-pages/VTKNamedColorPatches.html) showing the available colors in vtkNamedColors.

It also shows how to select the text color based on luminance. In this case, Digital CCIR601 is used, which gives less weight to the red and blue components of a color.

In the class HTMLTableMaker there are four methods for generating HTML pages:

- **MakeCombinedColorPage()** -- Makes an indexed page with all the colors and names in the class vtkNamedColors *(the default)*.
- **MakeWebColorPage()** -- Makes a page of just the colors and names known to Web Browsers.
- **MakeVTKColorPage()** -- Makes a page of colors and names corresponding to to additional colors commonly used in VTK.
- **MakeSynonymColorPage()** -- Makes a page of color names and their synonyms.
