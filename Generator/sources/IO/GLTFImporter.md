### Description

The example uses vtkGLTFImporter to import a scene from a *gltf* file.  [glTF](https://en.wikipedia.org/wiki/GlTF) (derivative short form of GL Transmission Format) is a file format for 3D scenes and models using the JSON standard. It is an API-neutral runtime asset delivery format developed by the Khronos Group 3D Formats Working Group. It was announced at HTML5DevConf 2016.

There are many sources of *glTF* file including:

* [SketchLab](https://sketchfab.com/3d-models?features=downloadable&sort_by=-likeCount). To download you will need to signup. There are also non-free models offered here.
* [Khronos Group](https://github.com/KhronosGroup/glTF-Sample-Models)

!!! example "usage"
    GLTFImporter FlightHelmet.gltf

!!! warning
    When you run the example, be sure to specify the full pathname of the .glTF file. There is currently a bug in the vtkGLTFImporter. For example, if your home directry is /home/janedoe the the full pathname of the FlightHelmet data is /home/janedoe/VTKExamples/src/Testing/Data/gltf/FlightHelmet/FlightHelmet.gltf.

!!! info
    The original data can be found here: [FlightHelmet Data](https://github.com/KhronosGroup/glTF-Sample-Models/tree/master/2.0/FlightHelmet)). Be sure to download all of the files.
