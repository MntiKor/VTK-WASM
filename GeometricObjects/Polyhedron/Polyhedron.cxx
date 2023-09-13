#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkDataSetMapper.h>
#include <vtkIdList.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridWriter.h>

int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;

  // Create the polyhedron, a cube.
  vtkIdType pointIds[8] = {0, 1, 2, 3, 4, 5, 6, 7};

  vtkNew<vtkPoints> points;
  points->InsertNextPoint(-1.0, -1.0, -1.0);
  points->InsertNextPoint(1.0, -1.0, -1.0);
  points->InsertNextPoint(1.0, 1.0, -1.0);
  points->InsertNextPoint(-1.0, 1.0, -1.0);
  points->InsertNextPoint(-1.0, -1.0, 1.0);
  points->InsertNextPoint(1.0, -1.0, 1.0);
  points->InsertNextPoint(1.0, 1.0, 1.0);
  points->InsertNextPoint(-1.0, 1.0, 1.0);

  vtkNew<vtkIdList> faces;
  vtkIdType face0[4] = {0, 3, 2, 1};
  vtkIdType face1[4] = {0, 4, 7, 3};
  vtkIdType face2[4] = {4, 5, 6, 7};
  vtkIdType face3[4] = {5, 1, 2, 6};
  vtkIdType face4[4] = {0, 1, 5, 4};
  vtkIdType face5[4] = {2, 3, 7, 6};

  auto addFace = [&](const vtkIdType face[4]) {
    faces->InsertNextId(4);
    for (int i = 0; i < 4; ++i)
    {
      faces->InsertNextId(face[i]);
    }
  };

  addFace(face0);
  addFace(face1);
  addFace(face2);
  addFace(face3);
  addFace(face4);
  addFace(face5);

  vtkNew<vtkUnstructuredGrid> ugrid;
  ugrid->SetPoints(points);
  ugrid->InsertNextCell(VTK_POLYHEDRON, 8, pointIds, 6, faces->GetPointer(0));

  // Here we write out the cube.
  vtkNew<vtkXMLUnstructuredGridWriter> writer;
  writer->SetInputData(ugrid);
  writer->SetFileName("polyhedron.vtu");
  writer->SetDataModeToAscii();
  writer->Update();

  // Create a mapper and actor
  vtkNew<vtkDataSetMapper> mapper;
  mapper->SetInputData(ugrid);

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(colors->GetColor3d("Silver").GetData());

  // Visualize.
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetWindowName("Polyhedron");
  renderWindow->AddRenderer(renderer);
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("Salmon").GetData());
  renderer->ResetCamera();
  renderer->GetActiveCamera()->Azimuth(30);
  renderer->GetActiveCamera()->Elevation(30);
  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
