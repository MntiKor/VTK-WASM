#include <vtkActor.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOpenVRRenderWindow.h>
#include <vtkOpenVRRenderWindowInteractor.h>
#include <vtkOpenVRRenderer.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkSphereSource.h>

int main(int, char*[])
{
  // Create a Sphere
  vtkNew<vtkSphereSource> sphereSource;
  sphereSource->SetCenter(0.0, 0.0, 0.0);
  sphereSource->SetRadius(5.0);

  // Create a Mapper
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(sphereSource->GetOutputPort());

  // Create an Actor
  vtkNew<vtkNamedColors> colors;
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

  // Create a Renderer, RenderWindow, RenderWindowInteractor
  vtkNew<vtkOpenVRRenderer> renderer;
  vtkNew<vtkOpenVRRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("OpenVRSphere");
  vtkNew<vtkOpenVRRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  // Add actor to the scene
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("ForestGreen").GetData());
  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
