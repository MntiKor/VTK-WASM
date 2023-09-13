#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCellArray.h>
#include <vtkDataSetMapper.h>
#include <vtkMCubesReader.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOutlineFilter.h>
#include <vtkPolyData.h>
#include <vtkPolyDataConnectivityFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

#ifdef VTK_CELL_ARRAY_V2
#include <vtkCellArrayIterator.h>
#endif // VTK_CELL_ARRAY_V2

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " filename [noConnectivity]"
              << std::endl;
    std::cout << "where: filename is pine_root.tri." << std::endl;
    std::cout << "       if noConnectivity is nonzero then the connectivity "
                 "filter is ignored."
              << std::endl;
    return EXIT_FAILURE;
  }

  /**
   * Count the triangles in the polydata.
   * @param pd: vtkPolyData.
   * @return The number of triangles.
   */
  auto NumberofTriangles = [](vtkPolyData* pd) {
    vtkCellArray* cells = pd->GetPolys();
    vtkIdType npts = 0;
    auto numOfTriangles = 0;
#ifdef VTK_CELL_ARRAY_V2

    // Newer versions of vtkCellArray prefer local iterators:
    auto cellIter = vtk::TakeSmartPointer(cells->NewIterator());
    for (cellIter->GoToFirstCell(); !cellIter->IsDoneWithTraversal();
         cellIter->GoToNextCell())
    {
      auto cell = cellIter->GetCurrentCell();
      if (cell == nullptr)
      {
        break;
      }
      // If a cell has three points it is a triangle.
      if (cell->GetNumberOfIds() == 3)
      {
        numOfTriangles++;
      }
    }

#else  // VTK_CELL_ARRAY_V2

    // Older implementations of vtkCellArray use internal iterator APIs (not
    // thread safe):
    vtkIdType* pts;
    for (auto i = 0; i < pd->GetNumberOfPolys(); ++i)
    {
      int c = cells->GetNextCell(npts, pts);
      if (c == 0)
      {
        break;
      }
      // If a cell has three points it is a triangle.
      if (npts == 3)
      {
        numOfTriangles++;
      }
    }
#endif // VTK_CELL_ARRAY_V2

    return numOfTriangles;
  };

  std::string fileName = argv[1];
  auto noConnectivity = false;
  if (argc > 2)
  {
    noConnectivity = atoi(argv[2]) != 0;
  }

  vtkNew<vtkNamedColors> colors;

  // Create the pipeline.
  vtkNew<vtkMCubesReader> reader;
  reader->SetFileName(fileName.c_str());
  reader->FlipNormalsOff();
  if (!noConnectivity)
  {
    reader->Update();
    std::cout << "Before Connectivity there are: "
              << NumberofTriangles(reader->GetOutput()) << " triangles."
              << std::endl;
  }

  vtkNew<vtkPolyDataConnectivityFilter> connect;
  connect->SetInputConnection(reader->GetOutputPort());
  connect->SetExtractionModeToLargestRegion();
  if (!noConnectivity)
  {
    connect->Update();
    std::cout << "After Connectivity there are:  "
              << NumberofTriangles(
                     dynamic_cast<vtkPolyData*>(connect->GetOutput()))
              << " triangles." << std::endl;
  }

  vtkNew<vtkDataSetMapper> isoMapper;
  if (noConnectivity)
  {
    isoMapper->SetInputConnection(reader->GetOutputPort());
  }
  else
  {
    isoMapper->SetInputConnection(connect->GetOutputPort());
  }
  isoMapper->ScalarVisibilityOff();
  vtkNew<vtkActor> isoActor;
  isoActor->SetMapper(isoMapper);
  isoActor->GetProperty()->SetColor(colors->GetColor3d("raw_sienna").GetData());

  // Get an outline of the data set for context.
  vtkNew<vtkOutlineFilter> outline;
  outline->SetInputConnection(reader->GetOutputPort());
  vtkNew<vtkPolyDataMapper> outlineMapper;
  outlineMapper->SetInputConnection(outline->GetOutputPort());
  vtkNew<vtkActor> outlineActor;
  outlineActor->SetMapper(outlineMapper);
  outlineActor->GetProperty()->SetColor(colors->GetColor3d("Black").GetData());

  // Create the Renderer, RenderWindow and RenderWindowInteractor.
  vtkNew<vtkRenderer> ren;
  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(ren);
  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);

  // Add the actors to the renderer, set the background and size.
  ren->AddActor(outlineActor);
  ren->AddActor(isoActor);
  // renWin->SetSize(750, 750);
  renWin->SetSize(512, 512);
  renWin->SetWindowName("PineRootConnectivity");

  ren->SetBackground(colors->GetColor3d("SlateGray").GetData());

  vtkCamera* cam = ren->GetActiveCamera();
  cam->SetFocalPoint(40.6018, 37.2813, 50.1953);
  cam->SetPosition(40.6018, -280.533, 47.0172);
  cam->ComputeViewPlaneNormal();
  cam->SetClippingRange(26.1073, 1305.36);
  cam->SetViewAngle(20.9219);
  cam->SetViewUp(0.0, 0.0, 1.0);

  iren->Initialize();
  renWin->Render();
  iren->Start();

  return EXIT_SUCCESS;
}
