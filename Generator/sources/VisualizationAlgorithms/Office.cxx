#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPointSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkStreamTracer.h>
#include <vtkStructuredGrid.h>
#include <vtkStructuredGridGeometryFilter.h>
#include <vtkStructuredGridOutlineFilter.h>
#include <vtkStructuredGridReader.h>

#include <array>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0]
              << " filename [center] e.g. office.binary.vtk 3" << std::endl;
    std::cout
        << "Demonstrate the use of vtkPointSource to generate streamlines."
        << std::endl;
    std::cout << "Where: " << std::endl;
    std::cout << "       filename: office.binary.vtk" << std::endl;
    std::cout << "       center: An optional parameter choosing the center for "
                 "the seeds."
              << std::endl;
    std::cout
        << "               0 - Corresponds to Fig 9-47(a) in the VTK textbook."
        << std::endl;
    std::cout << "               1 - A slight shift to the left." << std::endl;
    std::cout << "               2 - A slight shift to the upper left. (from "
                 "the Original code)."
              << std::endl;
    std::cout
        << "               3 - The default, a slight shift to the upper left."
        << std::endl;
    std::cout << "                   Roughly corresponds to Fig 9-47(b) in the "
                 "VTK textbook."
              << std::endl;
    return EXIT_FAILURE;
  }

  auto center = 3;
  if (argc > 2)
  {
    center = std::abs(atoi(argv[2]));
  }
  // These are the centers for the streamline seed.
  std::vector<std::array<double, 3>> seedCenters{
      {0.0, 2.1, 0.5}, {0.1, 2.1, 0.5}, {0.1, 2.7, 0.5}, {0.08, 2.7, 0.5}};
  center = (center < static_cast<int>(seedCenters.size()))
      ? center
      : static_cast<int>(seedCenters.size()) - 1;

  vtkNew<vtkNamedColors> colors;
  std::array<double, 3> tableTopColor = {0.59, 0.427, 0.392};
  std::array<double, 3> filingCabinetColor = {0.8, 0.8, 0.6};
  std::array<double, 3> bookShelfColor = {0.8, 0.8, 0.6};
  std::array<double, 3> windowColor = {0.3, 0.3, 0.5};
  colors->SetColor("TableTop", tableTopColor.data());
  colors->SetColor("FilingCabinet", filingCabinetColor.data());
  colors->SetColor("BookShelf", bookShelfColor.data());
  colors->SetColor("Window", windowColor.data());

  // We read a data file that represents a CFD analysis of airflow in an office
  // (with ventilation and a burning cigarette).
  vtkNew<vtkStructuredGridReader> reader;
  reader->SetFileName(argv[1]);

  // Create the scene.
  // We generate a whole bunch of planes which correspond to
  // the geometry in the analysis; tables, bookshelves and so on.
  vtkNew<vtkStructuredGridGeometryFilter> table1;
  table1->SetInputConnection(reader->GetOutputPort());
  table1->SetExtent(11, 15, 7, 9, 8, 8);
  vtkNew<vtkPolyDataMapper> mapTable1;
  mapTable1->SetInputConnection(table1->GetOutputPort());
  mapTable1->ScalarVisibilityOff();

  vtkNew<vtkActor> table1Actor;
  table1Actor->SetMapper(mapTable1);
  table1Actor->GetProperty()->SetColor(
      colors->GetColor3d("TableTop").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> table2;
  table2->SetInputConnection(reader->GetOutputPort());
  table2->SetExtent(11, 15, 10, 12, 8, 8);
  vtkNew<vtkPolyDataMapper> mapTable2;
  mapTable2->SetInputConnection(table2->GetOutputPort());
  mapTable2->ScalarVisibilityOff();
  vtkNew<vtkActor> table2Actor;
  table2Actor->SetMapper(mapTable2);
  table2Actor->GetProperty()->SetColor(
      colors->GetColor3d("TableTop").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> FilingCabinet1;
  FilingCabinet1->SetInputConnection(reader->GetOutputPort());
  FilingCabinet1->SetExtent(15, 15, 7, 9, 0, 8);
  vtkNew<vtkPolyDataMapper> mapFilingCabinet1;
  mapFilingCabinet1->SetInputConnection(FilingCabinet1->GetOutputPort());
  mapFilingCabinet1->ScalarVisibilityOff();
  vtkNew<vtkActor> FilingCabinet1Actor;
  FilingCabinet1Actor->SetMapper(mapFilingCabinet1);
  FilingCabinet1Actor->GetProperty()->SetColor(
      colors->GetColor3d("FilingCabinet").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> FilingCabinet2;
  FilingCabinet2->SetInputConnection(reader->GetOutputPort());
  FilingCabinet2->SetExtent(15, 15, 10, 12, 0, 8);
  vtkNew<vtkPolyDataMapper> mapFilingCabinet2;
  mapFilingCabinet2->SetInputConnection(FilingCabinet2->GetOutputPort());
  mapFilingCabinet2->ScalarVisibilityOff();
  vtkNew<vtkActor> FilingCabinet2Actor;
  FilingCabinet2Actor->SetMapper(mapFilingCabinet2);
  FilingCabinet2Actor->GetProperty()->SetColor(
      colors->GetColor3d("FilingCabinet").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> bookshelf1Top;
  bookshelf1Top->SetInputConnection(reader->GetOutputPort());
  bookshelf1Top->SetExtent(13, 13, 0, 4, 0, 11);
  vtkNew<vtkPolyDataMapper> mapBookshelf1Top;

  mapBookshelf1Top->SetInputConnection(bookshelf1Top->GetOutputPort());
  mapBookshelf1Top->ScalarVisibilityOff();
  vtkNew<vtkActor> bookshelf1TopActor;
  bookshelf1TopActor->SetMapper(mapBookshelf1Top);
  bookshelf1TopActor->GetProperty()->SetColor(
      colors->GetColor3d("BookShelf").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> bookshelf1Bottom;
  bookshelf1Bottom->SetInputConnection(reader->GetOutputPort());
  bookshelf1Bottom->SetExtent(20, 20, 0, 4, 0, 11);
  vtkNew<vtkPolyDataMapper> mapBookshelf1Bottom;
  mapBookshelf1Bottom->SetInputConnection(bookshelf1Bottom->GetOutputPort());
  mapBookshelf1Bottom->ScalarVisibilityOff();
  vtkNew<vtkActor> bookshelf1BottomActor;
  bookshelf1BottomActor->SetMapper(mapBookshelf1Bottom);
  bookshelf1BottomActor->GetProperty()->SetColor(
      colors->GetColor3d("BookShelf").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> bookshelf1Front;
  bookshelf1Front->SetInputConnection(reader->GetOutputPort());
  bookshelf1Front->SetExtent(13, 20, 0, 0, 0, 11);
  vtkNew<vtkPolyDataMapper> mapBookshelf1Front;
  mapBookshelf1Front->SetInputConnection(bookshelf1Front->GetOutputPort());
  mapBookshelf1Front->ScalarVisibilityOff();
  vtkNew<vtkActor> bookshelf1FrontActor;
  bookshelf1FrontActor->SetMapper(mapBookshelf1Front);
  bookshelf1FrontActor->GetProperty()->SetColor(
      colors->GetColor3d("BookShelf").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> bookshelf1Back;
  bookshelf1Back->SetInputConnection(reader->GetOutputPort());
  bookshelf1Back->SetExtent(13, 20, 4, 4, 0, 11);
  vtkNew<vtkPolyDataMapper> mapBookshelf1Back;
  mapBookshelf1Back->SetInputConnection(bookshelf1Back->GetOutputPort());
  mapBookshelf1Back->ScalarVisibilityOff();
  vtkNew<vtkActor> bookshelf1BackActor;
  bookshelf1BackActor->SetMapper(mapBookshelf1Back);
  bookshelf1BackActor->GetProperty()->SetColor(
      colors->GetColor3d("BookShelf").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> bookshelf1LHS;
  bookshelf1LHS->SetInputConnection(reader->GetOutputPort());
  bookshelf1LHS->SetExtent(13, 20, 0, 4, 0, 0);
  vtkNew<vtkPolyDataMapper> mapBookshelf1LHS;
  mapBookshelf1LHS->SetInputConnection(bookshelf1LHS->GetOutputPort());
  mapBookshelf1LHS->ScalarVisibilityOff();
  vtkNew<vtkActor> bookshelf1LHSActor;
  bookshelf1LHSActor->SetMapper(mapBookshelf1LHS);
  bookshelf1LHSActor->GetProperty()->SetColor(
      colors->GetColor3d("BookShelf").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> bookshelf1RHS;
  bookshelf1RHS->SetInputConnection(reader->GetOutputPort());
  bookshelf1RHS->SetExtent(13, 20, 0, 4, 11, 11);
  vtkNew<vtkPolyDataMapper> mapBookshelf1RHS;
  mapBookshelf1RHS->SetInputConnection(bookshelf1RHS->GetOutputPort());
  mapBookshelf1RHS->ScalarVisibilityOff();
  vtkNew<vtkActor> bookshelf1RHSActor;
  bookshelf1RHSActor->SetMapper(mapBookshelf1RHS);
  bookshelf1RHSActor->GetProperty()->SetColor(
      colors->GetColor3d("BookShelf").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> bookshelf2Top;
  bookshelf2Top->SetInputConnection(reader->GetOutputPort());
  bookshelf2Top->SetExtent(13, 13, 15, 19, 0, 11);
  vtkNew<vtkPolyDataMapper> mapBookshelf2Top;
  mapBookshelf2Top->SetInputConnection(bookshelf2Top->GetOutputPort());
  mapBookshelf2Top->ScalarVisibilityOff();
  vtkNew<vtkActor> bookshelf2TopActor;
  bookshelf2TopActor->SetMapper(mapBookshelf2Top);
  bookshelf2TopActor->GetProperty()->SetColor(
      colors->GetColor3d("BookShelf").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> bookshelf2Bottom;
  bookshelf2Bottom->SetInputConnection(reader->GetOutputPort());
  bookshelf2Bottom->SetExtent(20, 20, 15, 19, 0, 11);
  vtkNew<vtkPolyDataMapper> mapBookshelf2Bottom;
  mapBookshelf2Bottom->SetInputConnection(bookshelf2Bottom->GetOutputPort());
  mapBookshelf2Bottom->ScalarVisibilityOff();
  vtkNew<vtkActor> bookshelf2BottomActor;
  bookshelf2BottomActor->SetMapper(mapBookshelf2Bottom);
  bookshelf2BottomActor->GetProperty()->SetColor(
      colors->GetColor3d("BookShelf").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> bookshelf2Front;
  bookshelf2Front->SetInputConnection(reader->GetOutputPort());
  bookshelf2Front->SetExtent(13, 20, 15, 15, 0, 11);
  vtkNew<vtkPolyDataMapper> mapBookshelf2Front;
  mapBookshelf2Front->SetInputConnection(bookshelf2Front->GetOutputPort());
  mapBookshelf2Front->ScalarVisibilityOff();
  vtkNew<vtkActor> bookshelf2FrontActor;

  bookshelf2FrontActor->SetMapper(mapBookshelf2Front);
  bookshelf2FrontActor->GetProperty()->SetColor(
      colors->GetColor3d("BookShelf").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> bookshelf2Back;
  bookshelf2Back->SetInputConnection(reader->GetOutputPort());
  bookshelf2Back->SetExtent(13, 20, 19, 19, 0, 11);
  vtkNew<vtkPolyDataMapper> mapBookshelf2Back;
  mapBookshelf2Back->SetInputConnection(bookshelf2Back->GetOutputPort());
  mapBookshelf2Back->ScalarVisibilityOff();
  vtkNew<vtkActor> bookshelf2BackActor;
  bookshelf2BackActor->SetMapper(mapBookshelf2Back);
  bookshelf2BackActor->GetProperty()->SetColor(
      colors->GetColor3d("BookShelf").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> bookshelf2LHS;

  bookshelf2LHS->SetInputConnection(reader->GetOutputPort());
  bookshelf2LHS->SetExtent(13, 20, 15, 19, 0, 0);
  vtkNew<vtkPolyDataMapper> mapBookshelf2LHS;
  mapBookshelf2LHS->SetInputConnection(bookshelf2LHS->GetOutputPort());
  mapBookshelf2LHS->ScalarVisibilityOff();
  vtkNew<vtkActor> bookshelf2LHSActor;
  bookshelf2LHSActor->SetMapper(mapBookshelf2LHS);
  bookshelf2LHSActor->GetProperty()->SetColor(
      colors->GetColor3d("BookShelf").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> bookshelf2RHS;
  bookshelf2RHS->SetInputConnection(reader->GetOutputPort());
  bookshelf2RHS->SetExtent(13, 20, 15, 19, 11, 11);
  vtkNew<vtkPolyDataMapper> mapBookshelf2RHS;
  mapBookshelf2RHS->SetInputConnection(bookshelf2RHS->GetOutputPort());
  mapBookshelf2RHS->ScalarVisibilityOff();
  vtkNew<vtkActor> bookshelf2RHSActor;
  bookshelf2RHSActor->SetMapper(mapBookshelf2RHS);
  bookshelf2RHSActor->GetProperty()->SetColor(
      colors->GetColor3d("BookShelf").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> window;
  window->SetInputConnection(reader->GetOutputPort());
  window->SetExtent(20, 20, 6, 13, 10, 13);
  vtkNew<vtkPolyDataMapper> mapWindow;

  mapWindow->SetInputConnection(window->GetOutputPort());
  mapWindow->ScalarVisibilityOff();
  vtkNew<vtkActor> windowActor;
  windowActor->SetMapper(mapWindow);
  windowActor->GetProperty()->SetColor(colors->GetColor3d("Window").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> outlet;
  outlet->SetInputConnection(reader->GetOutputPort());
  outlet->SetExtent(0, 0, 9, 10, 14, 16);
  vtkNew<vtkPolyDataMapper> mapOutlet;
  mapOutlet->SetInputConnection(outlet->GetOutputPort());
  mapOutlet->ScalarVisibilityOff();
  vtkNew<vtkActor> outletActor;
  outletActor->SetMapper(mapOutlet);
  outletActor->GetProperty()->SetColor(
      colors->GetColor3d("lamp_black").GetData());

  vtkNew<vtkStructuredGridGeometryFilter> inlet;
  inlet->SetInputConnection(reader->GetOutputPort());
  inlet->SetExtent(0, 0, 9, 10, 0, 6);
  vtkNew<vtkPolyDataMapper> mapInlet;
  mapInlet->SetInputConnection(inlet->GetOutputPort());
  mapInlet->ScalarVisibilityOff();
  vtkNew<vtkActor> inletActor;
  inletActor->SetMapper(mapInlet);
  inletActor->GetProperty()->SetColor(
      colors->GetColor3d("lamp_black").GetData());

  vtkNew<vtkStructuredGridOutlineFilter> outline;
  outline->SetInputConnection(reader->GetOutputPort());
  vtkNew<vtkPolyDataMapper> mapOutline;
  mapOutline->SetInputConnection(outline->GetOutputPort());
  vtkNew<vtkActor> outlineActor;
  outlineActor->SetMapper(mapOutline);
  outlineActor->GetProperty()->SetColor(colors->GetColor3d("Black").GetData());

  // Create the source for the streamtubes.
  vtkNew<vtkPointSource> seeds;
  seeds->SetRadius(0.075);
  seeds->SetCenter(seedCenters[center].data());
  seeds->SetNumberOfPoints(25);
  vtkNew<vtkStreamTracer> streamers;
  streamers->SetInputConnection(reader->GetOutputPort());
  streamers->SetSourceConnection(seeds->GetOutputPort());
  streamers->SetMaximumPropagation(500);
  streamers->SetMinimumIntegrationStep(0.1);
  streamers->SetMaximumIntegrationStep(1.0);
  streamers->SetInitialIntegrationStep(0.2);
  streamers->SetIntegratorType(2);
  streamers->Update();
  vtkNew<vtkPolyDataMapper> mapStreamers;
  mapStreamers->SetInputConnection(streamers->GetOutputPort());
  mapStreamers->SetScalarRange(
      reader->GetOutput()->GetPointData()->GetScalars()->GetRange());
  vtkNew<vtkActor> streamersActor;
  streamersActor->SetMapper(mapStreamers);

  // Now create the usual graphics stuff.
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  vtkNew<vtkRenderWindowInteractor> interactor;
  interactor->SetRenderWindow(renderWindow);

  renderer->AddActor(table1Actor);
  renderer->AddActor(table2Actor);
  renderer->AddActor(FilingCabinet1Actor);
  renderer->AddActor(FilingCabinet2Actor);
  renderer->AddActor(bookshelf1TopActor);
  renderer->AddActor(bookshelf1BottomActor);
  renderer->AddActor(bookshelf1FrontActor);
  renderer->AddActor(bookshelf1BackActor);
  renderer->AddActor(bookshelf1LHSActor);
  renderer->AddActor(bookshelf1RHSActor);
  renderer->AddActor(bookshelf2TopActor);
  renderer->AddActor(bookshelf2BottomActor);
  renderer->AddActor(bookshelf2FrontActor);
  renderer->AddActor(bookshelf2BackActor);
  renderer->AddActor(bookshelf2LHSActor);
  renderer->AddActor(bookshelf2RHSActor);
  renderer->AddActor(windowActor);
  renderer->AddActor(outletActor);
  renderer->AddActor(inletActor);
  renderer->AddActor(outlineActor);
  renderer->AddActor(streamersActor);

  renderer->SetBackground(colors->GetColor3d("SlateGray").GetData());

  // Here we specify a particular view.
  vtkNew<vtkCamera> aCamera;
  aCamera->SetClippingRange(0.726079, 36.3039);
  aCamera->SetFocalPoint(2.43584, 2.15046, 1.11104);
  aCamera->SetPosition(-4.76183, -10.4426, 3.17203);
  aCamera->SetViewUp(0.0511273, 0.132773, 0.989827);
  aCamera->SetViewAngle(18.604);
  aCamera->Zoom(1.2);

  renderer->SetActiveCamera(aCamera);

  renderWindow->SetSize(640, 400);
  renderWindow->SetWindowName("Office");

  renderWindow->Render();
  interactor->Start();

  return EXIT_SUCCESS;
}
