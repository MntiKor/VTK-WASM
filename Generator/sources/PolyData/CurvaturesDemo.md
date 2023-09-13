### Description

**How to get the Gaussian and Mean curvatures of a surface.**

Two different surfaces are used in this demonstration with each surface coloured according to its Gaussian and Mean curvatures.

- The first surface is a superquadric surface, this demonstrates the use of extra filters that are needed to get a nice smooth surface.

- The second surface is a parametric surface, in this case the surface has already been triangulated so no extra processing is necessary.

In order to get a nice coloured image, a vtkColorTransferFunction has been used to generate a set of colours for the vtkLookupTable tables. We have used a diverging colour space.
Because of the symmetry of the ranges selected for the lookup tables, the white colouration represents a midpoint value whilst the blue represents values less than the midpoint value and orange represents colours greater than the midpoint value.
  
In the case of the Random Hills Gaussian curvature surface, this colouration shows the nature of the surface quite nicely. The blue areas are saddle points (negative Gaussian curvature) and the orange areas have a positive Gaussian curvature.

In the case of the mean curvature the blue colouration represents negative curvature perpendicular to one of the principal axes.
