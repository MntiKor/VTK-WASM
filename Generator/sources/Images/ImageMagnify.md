### Description

vtkImageMagnify increases the dimensions of an image by integral magnification factors. It also adjusts the spacing of the pixels so that the magnified image covers the same region as the original image. To stretch the image, the pixel spacing needs to be adjusted. This example uses vtkImageChangeInformation to modify the magnified image's spacing. The result is an image that has increased numbers of pixels and is stretched in the magnified directions.
