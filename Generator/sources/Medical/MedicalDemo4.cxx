// Derived from VTK/Examples/Cxx/Medical4.cxx
// This example reads a volume dataset and displays it via volume rendering.
//

#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkMetaImageReader.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include <array>

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    cout << "Usage: " << argv[0] << "file.mhd e.g. FullHead.mhd" << endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkNamedColors> colors;

  std::array<unsigned char, 4> bkg{{51, 77, 102, 255}};
  colors->SetColor("BkgColor", bkg.data());

  // Create the renderer, the render window, and the interactor. The renderer
  // draws into the render window, the interactor enables mouse- and
  // keyboard-based interaction with the scene.
  vtkNew<vtkRenderer> ren;
  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(ren);
  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);

  // The following reader is used to read a series of 2D slices (images)
  // that compose the volume. The slice dimensions are set, and the
  // pixel spacing. The data Endianness must also be specified. The reader
  // uses the FilePrefix in combination with the slice number to construct
  // filenames using the format FilePrefix.%d. (In this case the FilePrefix
  // is the root name of the file: quarter.)
  vtkNew<vtkMetaImageReader> reader;
  reader->SetFileName(argv[1]);

  // The volume will be displayed by ray-cast alpha compositing.
  // A ray-cast mapper is needed to do the ray-casting.
  vtkNew<vtkFixedPointVolumeRayCastMapper> volumeMapper;
  volumeMapper->SetInputConnection(reader->GetOutputPort());

  // The color transfer function maps voxel intensities to colors.
  // It is modality-specific, and often anatomy-specific as well.
  // The goal is to one color for flesh (between 500 and 1000)
  // and another color for bone (1150 and over).
  vtkNew<vtkColorTransferFunction> volumeColor;
  volumeColor->AddRGBPoint(0, 0.0, 0.0, 0.0);
  volumeColor->AddRGBPoint(500, 240.0 / 255.0, 184.0 / 255.0, 160.0 / 255.0);
  volumeColor->AddRGBPoint(1000, 240.0 / 255.0, 184.0 / 255.0, 160.0 / 255.0);
  volumeColor->AddRGBPoint(1150, 1.0, 1.0, 240.0 / 255.0); // Ivory

  // The opacity transfer function is used to control the opacity
  // of different tissue types.
  vtkNew<vtkPiecewiseFunction> volumeScalarOpacity;
  volumeScalarOpacity->AddPoint(0, 0.00);
  volumeScalarOpacity->AddPoint(500, 0.15);
  volumeScalarOpacity->AddPoint(1000, 0.15);
  volumeScalarOpacity->AddPoint(1150, 0.85);

  // The gradient opacity function is used to decrease the opacity
  // in the "flat" regions of the volume while maintaining the opacity
  // at the boundaries between tissue types. The gradient is measured
  // as the amount by which the intensity changes over unit distance.
  // For most medical data, the unit distance is 1mm.
  vtkNew<vtkPiecewiseFunction> volumeGradientOpacity;
  volumeGradientOpacity->AddPoint(0, 0.0);
  volumeGradientOpacity->AddPoint(90, 0.5);
  volumeGradientOpacity->AddPoint(100, 1.0);

  // The VolumeProperty attaches the color and opacity functions to the
  // volume, and sets other volume properties. The interpolation should
  // be set to linear to do a high-quality rendering. The ShadeOn option
  // turns on directional lighting, which will usually enhance the
  // appearance of the volume and make it look more "3D". However,
  // the quality of the shading depends on how accurately the gradient
  // of the volume can be calculated, and for noisy data the gradient
  // estimation will be very poor. The impact of the shading can be
  // decreased by increasing the Ambient coefficient while decreasing
  // the Diffuse and Specular coefficient. To increase the impact
  // of shading, decrease the Ambient and increase the Diffuse and Specular.
  vtkNew<vtkVolumeProperty> volumeProperty;
  volumeProperty->SetColor(volumeColor);
  volumeProperty->SetScalarOpacity(volumeScalarOpacity);
  volumeProperty->SetGradientOpacity(volumeGradientOpacity);
  volumeProperty->SetInterpolationTypeToLinear();
  volumeProperty->ShadeOn();
  volumeProperty->SetAmbient(0.4);
  volumeProperty->SetDiffuse(0.6);
  volumeProperty->SetSpecular(0.2);

  // The vtkVolume is a vtkProp3D (like a vtkActor) and controls the position
  // and orientation of the volume in world coordinates.
  vtkNew<vtkVolume> volume;
  volume->SetMapper(volumeMapper);
  volume->SetProperty(volumeProperty);

  // Finally, add the volume to the renderer
  ren->AddViewProp(volume);

  // Set up an initial view of the volume. The focal point will be the
  // center of the volume, and the camera position will be 400mm to the
  // patient's left (which is our right).
  vtkCamera* camera = ren->GetActiveCamera();
  double* c = volume->GetCenter();
  camera->SetViewUp(0, 0, -1);
  camera->SetPosition(c[0], c[1] - 400, c[2]);
  camera->SetFocalPoint(c[0], c[1], c[2]);
  camera->Azimuth(30.0);
  camera->Elevation(30.0);

  // Set a background color for the renderer
  ren->SetBackground(colors->GetColor3d("BkgColor").GetData());

  // Increase the size of the render window
  renWin->SetSize(640, 480);
  renWin->SetWindowName("MedicalDemo4");

  // Interact with the data.
  renWin->Render();
  iren->Start();

  return EXIT_SUCCESS;
}
