#include <vtkActor.h>
#include <vtkConeSource.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkElevationFilter.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
// #include <vtkSphereSource.h>

#include <array>

namespace {
vtkNew<vtkDiscretizableColorTransferFunction> getCTF();

}

int main(int, char*[])
{
  std::array<unsigned char, 4> bkg{82, 87, 110, 255};
  vtkNew<vtkNamedColors> colors;
  colors->SetColor("ParaViewBkg", bkg.data());

  vtkNew<vtkRenderer> ren;
  ren->SetBackground(colors->GetColor3d("ParaViewBkg").GetData());
  vtkNew<vtkRenderWindow> renWin;
  renWin->SetSize(640, 480);
  renWin->SetWindowName("ColorMapToLUT");
  renWin->AddRenderer(ren);
  vtkNew<vtkRenderWindowInteractor> iRen;
  iRen->SetRenderWindow(renWin);

  vtkNew<vtkInteractorStyleTrackballCamera> style;
  iRen->SetInteractorStyle(style);

  // vtkNew<vtkSphereSource> sphere;
  // sphere->SetThetaResolution(64);
  // sphere->SetPhiResolution(32);
  // auto bounds = sphere->GetOutput()->GetBounds();

  vtkNew<vtkConeSource> cone;
  cone->SetResolution(6);
  cone->SetDirection(0, 1, 0);
  cone->SetHeight(1);
  cone->Update();
  auto bounds = cone->GetOutput()->GetBounds();

  vtkNew<vtkElevationFilter> elevation_filter;
  elevation_filter->SetLowPoint(0, bounds[2], 0);
  elevation_filter->SetHighPoint(0, bounds[3], 0);
  elevation_filter->SetInputConnection(cone->GetOutputPort());
  // elevation_filter->SetInputConnection(sphere->GetOutputPort());

  auto ctf = getCTF();

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(elevation_filter->GetOutputPort());
  mapper->SetLookupTable(ctf);
  mapper->SetColorModeToMapScalars();
  mapper->InterpolateScalarsBeforeMappingOn();

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

  ren->AddActor(actor);

  renWin->Render();
  iRen->Start();

  return EXIT_SUCCESS;
}

namespace {

vtkNew<vtkDiscretizableColorTransferFunction> getCTF()
{
  // name: Fast, creator: Francesca Samsel, file name: Fast.xml
  vtkNew<vtkDiscretizableColorTransferFunction> ctf;

  ctf->SetColorSpaceToLab();
  ctf->SetScaleToLinear();

  ctf->SetNanColor(0, 0, 0);

  ctf->AddRGBPoint(0, 0.08800000000000002, 0.18810000000000007, 0.55);
  ctf->AddRGBPoint(0.16144, 0.21989453864645603, 0.5170512023315895,
                   0.7093372214401806);
  ctf->AddRGBPoint(0.351671, 0.5048913252297864, 0.8647869538833338,
                   0.870502284878942);
  ctf->AddRGBPoint(0.501285, 1, 1, 0.83);
  ctf->AddRGBPoint(0.620051, 0.9418960444346476, 0.891455547964053,
                   0.5446035798119958);
  ctf->AddRGBPoint(0.835408342528245, 0.75, 0.44475, 0.255);
  ctf->AddRGBPoint(1, 0.56, 0.055999999999999994, 0.055999999999999994);

  ctf->SetNumberOfValues(7);
  ctf->DiscretizeOff();

  return ctf;
}

} // namespace
