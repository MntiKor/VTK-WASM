#include <vtkCamera.h>
#include <vtkDataSetMapper.h>
#include <vtkExtractSelection.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPointSource.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include <vtkSphereSource.h>
#include <vtkUnstructuredGrid.h>

// #include <array>

int main(int, char*[])
{

  vtkNew<vtkNamedColors> colors;

  // std::array<double, 4> bkgL = {{0.6, 0.5, 0.4, 1.0}};
  // std::array<double, 4> bkgC = {{0.3, 0.1, 0.4, 1.0}};
  // std::array<double, 4> bkgR = {{0.4, 0.5, 0.6, 1.0}};
  // colors->SetColor("leftBkg", bkgL.data());
  // colors->SetColor("centreBkg", bkgC.data());
  // colors->SetColor("rightBkg", bkgR.data());

  vtkNew<vtkPointSource> pointSource;
  pointSource->SetNumberOfPoints(50);
  pointSource->Update();

  std::cout << "There are " << pointSource->GetOutput()->GetNumberOfPoints()
            << " input points." << std::endl;

  vtkNew<vtkIdTypeArray> ids;
  ids->SetNumberOfComponents(1);

  // Set values.
  for (unsigned int i = 10; i < 20; i++)
  {
    ids->InsertNextValue(i);
  }

  vtkNew<vtkSelectionNode> selectionNode;
  selectionNode->SetFieldType(vtkSelectionNode::POINT);
  selectionNode->SetContentType(vtkSelectionNode::INDICES);
  selectionNode->SetSelectionList(ids);

  vtkNew<vtkSelection> selection;
  selection->AddNode(selectionNode);

  vtkNew<vtkExtractSelection> extractSelection;
  extractSelection->SetInputConnection(0, pointSource->GetOutputPort());
  extractSelection->SetInputData(1, selection);
  extractSelection->Update();

  // In selection.
  vtkNew<vtkUnstructuredGrid> selected;
  selected->ShallowCopy(extractSelection->GetOutput());

  std::cout << "There are " << selected->GetNumberOfPoints() << " points and ";
  std::cout << selected->GetNumberOfCells() << " cells in the selection."
            << std::endl;

  // Get points that are NOT in the selection.
  selectionNode->GetProperties()->Set(vtkSelectionNode::INVERSE(),
                                      1); // invert the selection
  extractSelection->Update();

  vtkNew<vtkUnstructuredGrid> notSelected;
  notSelected->ShallowCopy(extractSelection->GetOutput());

  std::cout << "There are " << notSelected->GetNumberOfPoints()
            << " points and ";
  std::cout << notSelected->GetNumberOfCells() << " cells NOT in the selection."
            << std::endl;

  vtkNew<vtkDataSetMapper> inputMapper;
  inputMapper->SetInputConnection(pointSource->GetOutputPort());
  vtkNew<vtkActor> inputActor;
  inputActor->SetMapper(inputMapper);
  inputActor->GetProperty()->SetColor(
      colors->GetColor3d("MidnightBlue").GetData());
  inputActor->GetProperty()->SetPointSize(5);

  vtkNew<vtkDataSetMapper> selectedMapper;
  selectedMapper->SetInputData(selected);

  vtkNew<vtkActor> selectedActor;
  selectedActor->SetMapper(selectedMapper);
  selectedActor->GetProperty()->SetColor(
      colors->GetColor3d("MidnightBlue").GetData());
  selectedActor->GetProperty()->SetPointSize(5);

  vtkNew<vtkDataSetMapper> notSelectedMapper;
  notSelectedMapper->SetInputData(notSelected);

  vtkNew<vtkActor> notSelectedActor;
  notSelectedActor->SetMapper(notSelectedMapper);
  notSelectedActor->GetProperty()->SetColor(
      colors->GetColor3d("MidnightBlue").GetData());
  notSelectedActor->GetProperty()->SetPointSize(5);

  // There will be one render window.
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetSize(900, 300);
  renderWindow->SetWindowName("ExtractSelection");

  // And one interactor.
  vtkNew<vtkRenderWindowInteractor> interactor;
  interactor->SetRenderWindow(renderWindow);

  // Define viewport ranges (xmin, ymin, xmax, ymax).
  double leftViewport[4] = {0.0, 0.0, 0.33, 1.0};
  double centerViewport[4] = {0.33, 0.0, .66, 1.0};
  double rightViewport[4] = {0.66, 0.0, 1.0, 1.0};

  // Create a camera for all renderers.
  vtkNew<vtkCamera> camera;

  // Setup the renderers.
  vtkNew<vtkRenderer> leftRenderer;
  renderWindow->AddRenderer(leftRenderer);
  leftRenderer->SetViewport(leftViewport);
  leftRenderer->SetBackground(colors->GetColor3d("BurlyWood").GetData());
  leftRenderer->SetActiveCamera(camera);

  vtkNew<vtkRenderer> centerRenderer;
  renderWindow->AddRenderer(centerRenderer);
  centerRenderer->SetViewport(centerViewport);
  centerRenderer->SetBackground(colors->GetColor3d("orchid_dark").GetData());
  centerRenderer->SetActiveCamera(camera);

  vtkNew<vtkRenderer> rightRenderer;
  renderWindow->AddRenderer(rightRenderer);
  rightRenderer->SetViewport(rightViewport);
  rightRenderer->SetBackground(colors->GetColor3d("CornflowerBlue").GetData());
  rightRenderer->SetActiveCamera(camera);

  leftRenderer->AddActor(inputActor);
  centerRenderer->AddActor(selectedActor);
  rightRenderer->AddActor(notSelectedActor);

  leftRenderer->ResetCamera();

  renderWindow->Render();
  interactor->Start();

  return EXIT_SUCCESS;
}
