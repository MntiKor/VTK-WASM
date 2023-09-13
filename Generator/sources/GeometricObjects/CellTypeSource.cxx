#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkCellData.h>
#include <vtkCellTypeSource.h>
#include <vtkCellTypes.h>
#include <vtkColorSeries.h>
#include <vtkDataSetMapper.h>
#include <vtkIntArray.h>
#include <vtkLookupTable.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkShrinkFilter.h>
#include <vtkTessellatorFilter.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>
#include <vtkUnstructuredGrid.h>

#include <map>

int main(int argc, char* argv[])
{
  std::string cellName = "vtkTetra";
  if (argc > 1)
  {
    cellName = std::string(argv[1]);
  }

  // Store cell class names in a map
  std::map<std::string, int> cellMap;
  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_LINE)] = VTK_LINE;
  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_QUADRATIC_EDGE)] =
      VTK_QUADRATIC_EDGE;
  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_CUBIC_LINE)] =
      VTK_CUBIC_LINE;

  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_TRIANGLE)] = VTK_TRIANGLE;
  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_QUADRATIC_TRIANGLE)] =
      VTK_QUADRATIC_TRIANGLE;
  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_QUAD)] = VTK_QUAD;
  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_QUADRATIC_QUAD)] =
      VTK_QUADRATIC_QUAD;

  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_TETRA)] = VTK_TETRA;
  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_HEXAHEDRON)] =
      VTK_HEXAHEDRON;
  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_WEDGE)] = VTK_WEDGE;
  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_PYRAMID)] = VTK_PYRAMID;
  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_QUADRATIC_WEDGE)] =
      VTK_QUADRATIC_WEDGE;
  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_QUADRATIC_PYRAMID)] =
      VTK_QUADRATIC_PYRAMID;
  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_QUADRATIC_HEXAHEDRON)] =
      VTK_QUADRATIC_HEXAHEDRON;
  cellMap[vtkCellTypes::GetClassNameFromTypeId(VTK_QUADRATIC_TETRA)] =
      VTK_QUADRATIC_TETRA;

  if (cellMap.find(std::string(cellName)) == cellMap.end())
  {
    std::cout << "Cell type " << cellName << " is not supported." << std::endl;
    return EXIT_FAILURE;
  }
  vtkNew<vtkCellTypeSource> source;
  source->SetCellType(cellMap[cellName]);
  source->Update();
  std::cout << "Cell: " << cellName << std::endl;

  vtkPoints* originalPoints = source->GetOutput()->GetPoints();
  vtkNew<vtkPoints> points;
  points->SetNumberOfPoints(source->GetOutput()->GetNumberOfPoints());
  vtkNew<vtkMinimalStandardRandomSequence> rng;
  rng->SetSeed(5070); // for testing
  for (auto i = 0; i < points->GetNumberOfPoints(); ++i)
  {
    double perturbation[3];
    for (auto j = 0; j < 3; ++j)
    {
      rng->Next();
      perturbation[j] = rng->GetRangeValue(-0.1, 0.1);
    }
    double currentPoint[3];
    originalPoints->GetPoint(i, currentPoint);
    points->SetPoint(i, currentPoint[0] + perturbation[0],
                     currentPoint[1] + perturbation[1],
                     currentPoint[2] + perturbation[2]);
  }
  source->GetOutput()->SetPoints(points);

  int numCells = source->GetOutput()->GetNumberOfCells();
  std::cout << "Number of cells: " << numCells << std::endl;
  vtkNew<vtkIntArray> idArray;
  idArray->SetNumberOfTuples(numCells);
  for (auto i = 0; i < numCells; ++i)
  {
    idArray->InsertTuple1(i, i + 1);
  }
  idArray->SetName("Ids");
  source->GetOutput()->GetCellData()->AddArray(idArray);
  source->GetOutput()->GetCellData()->SetActiveScalars("Ids");

  vtkNew<vtkShrinkFilter> shrink;
  shrink->SetInputConnection(source->GetOutputPort());
  shrink->SetShrinkFactor(.8);

  vtkNew<vtkTessellatorFilter> tessellate;
  tessellate->SetInputConnection(shrink->GetOutputPort());
  tessellate->SetMaximumNumberOfSubdivisions(3);

  // Create a lookup table to map cell data to colors
  vtkNew<vtkLookupTable> lut;

  vtkNew<vtkColorSeries> colorSeries;
  int seriesEnum = colorSeries->BREWER_QUALITATIVE_SET3;
  ;
  colorSeries->SetColorScheme(seriesEnum);
  colorSeries->BuildLookupTable(lut, colorSeries->ORDINAL);

  // Fill in a few known colors, the rest will be generated if needed
  vtkNew<vtkNamedColors> colors;

  // Create a mapper and actor
  vtkNew<vtkDataSetMapper> mapper;
  mapper->SetInputConnection(source->GetOutputPort());
  mapper->SetInputConnection(shrink->GetOutputPort());
  mapper->SetScalarRange(0, numCells + 1);
  mapper->SetLookupTable(lut);
  mapper->SetScalarModeToUseCellData();
  mapper->SetResolveCoincidentTopologyToPolygonOffset();
  if (source->GetCellType() == VTK_QUADRATIC_PYRAMID ||
      source->GetCellType() == VTK_QUADRATIC_WEDGE)
  {
    mapper->SetInputConnection(shrink->GetOutputPort());
  }
  else
  {
    mapper->SetInputConnection(tessellate->GetOutputPort());
  }
  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->EdgeVisibilityOn();
  //  actor->GetProperty()->SetLineWidth(3);

  vtkNew<vtkTextProperty> textProperty;
  textProperty->SetFontSize(20);
  textProperty->SetJustificationToCentered();
  textProperty->SetColor(colors->GetColor3d("Lamp_Black").GetData());

  vtkNew<vtkTextMapper> textMapper;
  textMapper->SetInput(cellName.c_str());
  textMapper->SetTextProperty(textProperty);

  vtkNew<vtkActor2D> textActor;
  textActor->SetMapper(textMapper);
  textActor->SetPosition(320, 20);

  // Create a renderer, render window, and interactor
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetWindowName("CellTypeSource");
  renderWindow->AddRenderer(renderer);
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  // Add the actors to the scene
  renderer->AddViewProp(textActor);
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("Silver").GetData());

  renderer->ResetCamera();
  renderer->GetActiveCamera()->Azimuth(30);
  renderer->GetActiveCamera()->Elevation(30);
  renderer->ResetCameraClippingRange();

  // Render and interact
  renderWindow->SetSize(640, 480);
  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
