#include <vtkActor.h>
#include <vtkGraphicsFactory.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPNGWriter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkWindowToImageFilter.h>

int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;

  // Setup offscreen rendering
  vtkNew<vtkGraphicsFactory> graphics_factory;
  graphics_factory->SetOffScreenOnlyMode(1);
  graphics_factory->SetUseMesaClasses(1);

  // Create a sphere
  vtkNew<vtkSphereSource> sphereSource;

  // Create a mapper and actor
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(sphereSource->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(colors->GetColor3d("White").GetData());

  // A renderer and render window
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetOffScreenRendering(1);
  renderWindow->AddRenderer(renderer);

  // Add the actors to the scene
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("SlateGray").GetData());

  renderWindow->Render();

  vtkNew<vtkWindowToImageFilter> windowToImageFilter;
  windowToImageFilter->SetInput(renderWindow);
  windowToImageFilter->Update();

  vtkNew<vtkPNGWriter> writer;
  writer->SetFileName("screenshot.png");
  writer->SetInputConnection(windowToImageFilter->GetOutputPort());
  writer->Write();

  return EXIT_SUCCESS;
}
