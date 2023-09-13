#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkCutter.h>
#include <vtkKochanekSpline.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkSplineFilter.h>
#include <vtkStripper.h>
#include <vtkTubeFilter.h>
#include <vtkXMLPolyDataReader.h>

#ifdef VTK_CELL_ARRAY_V2
#include <vtkCellArrayIterator.h>
#endif // VTK_CELL_ARRAY_V2

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
  vtkSmartPointer<vtkPolyData> polyData;
  if (argc > 1)
  {
    vtkNew<vtkXMLPolyDataReader> reader;
    reader->SetFileName(argv[1]);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else
  {
    vtkNew<vtkSphereSource> modelSource;
    modelSource->Update();
    polyData = modelSource->GetOutput();
  }

  double length = polyData->GetLength();

  vtkNew<vtkPlane> plane;
  plane->SetNormal(0, 1, 1);
  plane->SetOrigin(polyData->GetCenter());

  vtkNew<vtkCutter> cutter;
  cutter->SetInputData(polyData);
  cutter->SetCutFunction(plane);
  cutter->GenerateValues(1, 0.0, 0.0);

  vtkNew<vtkNamedColors> colors;

  vtkNew<vtkPolyDataMapper> modelMapper;
  modelMapper->SetInputData(polyData);

  vtkNew<vtkActor> model;
  model->SetMapper(modelMapper);
  model->GetProperty()->SetColor(colors->GetColor3d("Tomato").GetData());
  model->GetProperty()->SetInterpolationToFlat();

  vtkNew<vtkStripper> stripper;
  stripper->SetInputConnection(cutter->GetOutputPort());

  vtkNew<vtkKochanekSpline> spline;
  spline->SetDefaultTension(.5);

  vtkNew<vtkSplineFilter> sf;
  sf->SetInputConnection(stripper->GetOutputPort());
  sf->SetSubdivideToSpecified();
  sf->SetNumberOfSubdivisions(50);
  sf->SetSpline(spline);
  sf->GetSpline()->ClosedOn();

  vtkNew<vtkTubeFilter> tubes;
  tubes->SetInputConnection(sf->GetOutputPort());
  tubes->SetNumberOfSides(8);
  tubes->SetRadius(length / 100.0);

  vtkNew<vtkPolyDataMapper> linesMapper;
  linesMapper->SetInputConnection(tubes->GetOutputPort());
  linesMapper->ScalarVisibilityOff();

  vtkNew<vtkActor> lines;
  lines->SetMapper(linesMapper);
  lines->GetProperty()->SetColor(colors->GetColor3d("Banana").GetData());

  vtkNew<vtkRenderer> renderer;
  renderer->UseHiddenLineRemovalOn();

  vtkNew<vtkRenderWindow> renderWindow;
  vtkNew<vtkRenderWindowInteractor> interactor;
  interactor->SetRenderWindow(renderWindow);

  // Add the actors to the renderer.
  renderer->AddActor(model);
  renderer->AddActor(lines);

  renderer->ResetCamera();
  renderer->SetBackground(colors->GetColor3d("SlateGray").GetData());
  renderer->GetActiveCamera()->Azimuth(300);
  renderer->GetActiveCamera()->Elevation(30);
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(640, 480);
  renderWindow->SetWindowName("FitSplineToCutterOutput");

  // This starts the event loop and as a side effect causes an initial
  // render.
  renderWindow->Render();
  interactor->Start();

  // Extract the lines from the polydata.
  vtkIdType numberOfLines = cutter->GetOutput()->GetNumberOfLines();

  std::cout << "-----------Lines without using vtkStripper" << std::endl;
  if (numberOfLines == 1)
  {
    std::cout << "There is " << numberOfLines << " line in the polydata"
              << std::endl;
  }
  else
  {
    std::cout << "There are " << numberOfLines << " lines in the polydata"
              << std::endl;
  }
  numberOfLines = stripper->GetOutput()->GetNumberOfLines();
  vtkPoints* points = stripper->GetOutput()->GetPoints();
  vtkCellArray* cells = stripper->GetOutput()->GetLines();

  std::cout << "-----------Lines using vtkStripper" << std::endl;
  if (numberOfLines == 1)
  {
    std::cout << "There is " << numberOfLines << " line in the polydata"
              << std::endl;
  }
  else
  {

    std::cout << "There are " << numberOfLines << " lines in the polydata"
              << std::endl;
  }

#ifdef VTK_CELL_ARRAY_V2

  // Newer versions of vtkCellArray prefer local iterators:
  auto cellIter = vtk::TakeSmartPointer(cells->NewIterator());
  for (cellIter->GoToFirstCell(); !cellIter->IsDoneWithTraversal();
       cellIter->GoToNextCell())
  {
    std::cout << "Line " << cellIter->GetCurrentCellId() << ":\n";

    vtkIdList* cell = cellIter->GetCurrentCell();
    for (vtkIdType i = 0; i < cell->GetNumberOfIds(); ++i)
    {
      double point[3];
      points->GetPoint(cell->GetId(i), point);
      std::cout << "\t(" << point[0] << ", " << point[1] << ", " << point[2]
                << ")" << std::endl;
    }
  }

#else // VTK_CELL_ARRAY_V2

  // Older implementations of vtkCellArray use internal iterator APIs (not
  // thread safe):
  vtkIdType* indices;
  vtkIdType numberOfPoints;
  unsigned int lineCount = 0;
  for (cells->InitTraversal(); cells->GetNextCell(numberOfPoints, indices);
       lineCount++)
  {
    std::cout << "Line " << lineCount << ": " << std::endl;
    for (vtkIdType i = 0; i < numberOfPoints; i++)
    {
      double point[3];
      points->GetPoint(indices[i], point);
      std::cout << "\t(" << point[0] << ", " << point[1] << ", " << point[2]
                << ")" << std::endl;
    }
  }

#endif // VTK_CELL_ARRAY_V2

  return EXIT_SUCCESS;
}
