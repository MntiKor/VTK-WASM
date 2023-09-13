#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkCellArray.h>
#include <vtkContourFilter.h>
#include <vtkDataArray.h>
#include <vtkDoubleArray.h>
#include <vtkLabeledDataMapper.h>
#include <vtkLookupTable.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPlaneSource.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkStripper.h>
#include <vtkTextProperty.h>
#include <vtkXMLPolyDataReader.h>

#include <iomanip>
#include <iostream>

#ifdef VTK_CELL_ARRAY_V2
#include <vtkCellArrayIterator.h>
#endif // VTK_CELL_ARRAY_V2

int main(int argc, char* argv[])
{
  vtkNew<vtkNamedColors> colors;

  int pointThreshold = 10;

  vtkSmartPointer<vtkPolyData> polyData;
  vtkNew<vtkContourFilter> contours;

  vtkNew<vtkMinimalStandardRandomSequence> randomSequence;
  randomSequence->SetSeed(1);

  // If a file is present, read it, otherwise generate some random
  // scalars on a plane.
  if (argc > 1)
  {
    vtkNew<vtkXMLPolyDataReader> reader;
    reader->SetFileName(argv[1]);
    reader->Update();

    double range[2];
    reader->GetOutput()->GetScalarRange(range);
    polyData = reader->GetOutput();

    std::cout << "range: " << range[0] << ", " << range[1] << std::endl;
    contours->SetValue(0, (range[1] + range[0]) / 2.0);

    contours->SetInputConnection(reader->GetOutputPort());
    if (argc == 3)
    {
      contours->SetValue(0, atof(argv[2]));
    }
    else if (argc == 4)
    {
      contours->SetValue(0, atof(argv[2]));
      contours->SetValue(1, atof(argv[3]));
    }
    else if (argc == 5)
    {
      contours->GenerateValues(atoi(argv[2]), atof(argv[3]), atof(argv[4]));
    }
  }
  else
  {
    vtkNew<vtkPlaneSource> plane;
    plane->SetXResolution(10);
    plane->SetYResolution(10);
    plane->Update();

    vtkNew<vtkDoubleArray> randomScalars;
    randomScalars->SetNumberOfComponents(1);
    randomScalars->SetName("Isovalues");
    for (int i = 0; i < plane->GetOutput()->GetNumberOfPoints(); i++)
    {
      randomScalars->InsertNextTuple1(
          randomSequence->GetRangeValue(-100.0, 100.0));
      randomSequence->Next();
    }
    plane->GetOutput()->GetPointData()->SetScalars(randomScalars);
    polyData = plane->GetOutput();
    contours->SetInputConnection(plane->GetOutputPort());
    contours->GenerateValues(5, -100, 100);
    pointThreshold = 0;
  }

  // Connect the segments of the contours into polylines.
  vtkNew<vtkStripper> contourStripper;
  contourStripper->SetInputConnection(contours->GetOutputPort());
  contourStripper->Update();

  auto numberOfContourLines = contourStripper->GetOutput()->GetNumberOfLines();

  std::cout << "There are " << numberOfContourLines << " contour lines."
            << std::endl;

  vtkPoints* points = contourStripper->GetOutput()->GetPoints();
  vtkCellArray* cells = contourStripper->GetOutput()->GetLines();
  vtkDataArray* scalars =
      contourStripper->GetOutput()->GetPointData()->GetScalars();

  // Create a polydata that contains point locations for the contour
  // line labels.
  vtkNew<vtkPolyData> labelPolyData;
  vtkNew<vtkPoints> labelPoints;
  vtkNew<vtkDoubleArray> labelScalars;
  labelScalars->SetNumberOfComponents(1);
  labelScalars->SetName("Isovalues");

#ifdef VTK_CELL_ARRAY_V2

  // Newer versions of vtkCellArray prefer local iterators:
  auto cellIter = vtk::TakeSmartPointer(cells->NewIterator());
  for (cellIter->GoToFirstCell(); !cellIter->IsDoneWithTraversal();
       cellIter->GoToNextCell())
  {
    vtkIdList* cell = cellIter->GetCurrentCell();
    if (cell->GetNumberOfIds() < pointThreshold)
    {
      continue;
    }
    std::cout << "Line " << cellIter->GetCurrentCellId() << ": " << std::endl;

    // Compute the point id to hold the label.
    // Mid point or a random point.
    // const vtkIdType samplePtIdx = cell->GetNumberOfIds() / 2;
    const vtkIdType samplePtIdx = static_cast<vtkIdType>(
        randomSequence->GetRangeValue(0, cell->GetNumberOfIds()));
    randomSequence->Next();

    auto midPointId = cell->GetId(samplePtIdx);

    double midPoint[3];
    points->GetPoint(midPointId, midPoint);
    std::cout << "\tmidPoint is " << midPointId << " with coordinate "
              << "(" << std::fixed << std::setprecision(2) << midPoint[0]
              << ", " << midPoint[1] << ", " << midPoint[2] << ")"
              << " and value " << scalars->GetTuple1(midPointId) << std::endl;
    labelPoints->InsertNextPoint(midPoint);
    labelScalars->InsertNextTuple1(scalars->GetTuple1(midPointId));
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
    if (numberOfPoints < pointThreshold)
    {
      continue;
    }
    std::cout << "Line " << lineCount << ": " << std::endl;

    // Compute the point id to hold the label.
    // Mid point or a random point.
    // auto midPointId = indices[numberOfPoints / 2];
    // auto midPointId =
    //    indices[static_cast<vtkIdType>(vtkMath::Random(0, numberOfPoints))];
    const vtkIdType samplePtIdx = static_cast<vtkIdType>(
        randomSequence->GetRangeValue(0, cell->GetNumberOfIds()));
    randomSequence->Next();

    double midPoint[3];
    points->GetPoint(midPointId, midPoint);
    std::cout << "\tmidPoint is " << midPointId << " with coordinate "
              << "(" << std::fixed << std::setprecision(2) << midPoint[0]
              << ", " << midPoint[1] << ", " << midPoint[2] << ")"
              << " and value " << scalars->GetTuple1(midPointId) << std::endl;
    labelPoints->InsertNextPoint(midPoint);
    labelScalars->InsertNextTuple1(scalars->GetTuple1(midPointId));
  }

#endif // VTK_CELL_ARRAY_V2

  labelPolyData->SetPoints(labelPoints);
  labelPolyData->GetPointData()->SetScalars(labelScalars);

  vtkNew<vtkPolyDataMapper> contourMapper;
  contourMapper->SetInputConnection(contourStripper->GetOutputPort());
  contourMapper->ScalarVisibilityOff();

  vtkNew<vtkActor> isolines;
  isolines->SetMapper(contourMapper);
  isolines->GetProperty()->SetColor(colors->GetColor3d("Black").GetData());
  isolines->GetProperty()->SetLineWidth(2);

  vtkNew<vtkLookupTable> surfaceLUT;
  surfaceLUT->SetRange(polyData->GetPointData()->GetScalars()->GetRange());
  surfaceLUT->Build();

  vtkNew<vtkPolyDataMapper> surfaceMapper;
  surfaceMapper->SetInputData(polyData);
  surfaceMapper->ScalarVisibilityOn();
  surfaceMapper->SetScalarRange(
      polyData->GetPointData()->GetScalars()->GetRange());
  surfaceMapper->SetLookupTable(surfaceLUT);

  vtkNew<vtkActor> surface;
  surface->SetMapper(surfaceMapper);

  // The labeled data mapper will place labels at the points.
  vtkNew<vtkLabeledDataMapper> labelMapper;
  labelMapper->SetFieldDataName("Isovalues");
  labelMapper->SetInputData(labelPolyData);
  labelMapper->SetLabelModeToLabelScalars();
  labelMapper->SetLabelFormat("%6.2f");
  labelMapper->GetLabelTextProperty()->SetColor(
      colors->GetColor3d("Gold").GetData());

  vtkNew<vtkActor2D> isolabels;
  isolabels->SetMapper(labelMapper);

  // Create a renderer and render window.
  vtkNew<vtkRenderer> renderer;

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(600, 600);
  renderWindow->SetWindowName("LabelContours");

  // Create an interactor
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  // Add the actors to the scene
  renderer->AddActor(surface);
  renderer->AddActor(isolines);
  renderer->AddActor(isolabels);
  renderer->SetBackground(colors->GetColor3d("DarkSlateGray").GetData());

  // Render the scene (lights and cameras are created automatically).
  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
