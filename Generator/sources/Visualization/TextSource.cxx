#include <vtkActor.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTextSource.h>

int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;

  // Create text
  vtkNew<vtkTextSource> textSource;
  textSource->SetText("Hello");
  textSource->SetForegroundColor(colors->GetColor3d("DarkSlateGray").GetData());
  textSource->SetBackgroundColor(colors->GetColor3d("NavajoWhite").GetData());
  // Turn off if you don't want the background drawn with the text.
  textSource->BackingOn();
  textSource->Update();

  // Create a mapper and actor.
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(textSource->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

  // Create a renderer, render window, and interactor.
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("TextSource");

  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  // Add the actor to the scene.
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("Bisque").GetData());

  // Render and interact.
  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
