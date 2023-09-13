#include <vtkActor.h>
#include <vtkColorSeries.h>
#include <vtkColorTransferFunction.h>
#include <vtkImageData.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkOpenVRRenderer.h>
#include <vtkPiecewiseFunction.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>

int main(int argc, char* argv[])
{
  int dim[3] = {10, 10, 10};
  double spc[3] = {0.05, 0.05, 0.05};
  vtkNew<vtkImageData> img;
  img->SetDimensions(dim);
  img->AllocateScalars(VTK_INT, 1);
  img->SetSpacing(spc);
  for (int x = 0; x < dim[0]; ++x)
    for (int y = 0; y < dim[1]; ++y)
      for (int z = 0; z < dim[2]; ++z)
      {
        img->SetScalarComponentFromDouble(x, y, z, 0, x);
      }
  vtkNew<vtkColorSeries> colors;
  colors->SetColorScheme(vtkColorSeries::BREWER_QUALITATIVE_SET3);
  vtkNew<vtkColorTransferFunction> ctf;
  for (int x = 0; x < dim[0]; ++x)
  {
    auto c = colors->GetColor(x);
    ctf->AddRGBPoint(x, c.GetRed() / 255.0, c.GetGreen() / 255.0,
                     c.GetBlue() / 255.0);
  }
  ctf->AddRGBPoint(dim[0], 1.0, 1.0, 1.0);
  ctf->Build();
  vtkNew<vtkSmartVolumeMapper> volMapper;
  vtkNew<vtkVolume> volume;
  vtkNew<vtkVolumeProperty> volProp;
  volMapper->SetBlendModeToComposite();
  volume->SetMapper(volMapper);
  volume->SetProperty(volProp);
  volume->SetVisibility(true);
  volMapper->SetInputData(img);
  volProp->SetColor(0, ctf);
  vtkNew<vtkPiecewiseFunction> otf;
  otf->AddPoint(0.0, 1.0);
  otf->AddPoint(dim[0], 1.0);
  volProp->SetScalarOpacity(0, otf);
  volProp->Modified();
  volProp->SetScalarOpacityUnitDistance(1);

  vtkNew<vtkOpenVRRenderWindow> renderWindow;
  renderWindow->Initialize();
  vtkNew<vtkOpenVRRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  vtkNew<vtkOpenVRRenderer> renderer;
  renderWindow->AddRenderer(renderer);
  vtkNew<vtkNamedColors> namedColors;
  renderer->SetBackground(namedColors->GetColor3d("ForestGreen").GetData());
  renderer->AddVolume(volume);
  volume->SetPosition(0, 0.5, 0);

  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
