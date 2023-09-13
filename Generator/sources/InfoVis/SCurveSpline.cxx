#include <vtkActor.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkParametricFunctionSource.h>
#include <vtkParametricSpline.h>
#include <vtkPointSource.h>
#include <vtkPoints.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSCurveSpline.h>

int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;

  vtkNew<vtkPointSource> pointSource;
  pointSource->SetNumberOfPoints(5);
  pointSource->Update();

  vtkPoints* points = pointSource->GetOutput()->GetPoints();

  vtkNew<vtkSCurveSpline> xSpline;
  vtkNew<vtkSCurveSpline> ySpline;
  vtkNew<vtkSCurveSpline> zSpline;

  vtkNew<vtkParametricSpline> spline;
  spline->SetXSpline(xSpline);
  spline->SetYSpline(ySpline);
  spline->SetZSpline(zSpline);
  spline->SetPoints(points);

  vtkNew<vtkParametricFunctionSource> functionSource;
  functionSource->SetParametricFunction(spline);
  functionSource->Update();

  // Setup actor and mapper
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(functionSource->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(colors->GetColor3d("Tomato").GetData());

  // Setup render window, renderer, and interactor
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("SCurveSpline");

  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("DarkSlateGray").GetData());

  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
