#include <vtkActor.h>
#include <vtkButterflySubdivisionFilter.h>
#include <vtkCamera.h>
#include <vtkLegendBoxActor.h>
#include <vtkLinearSubdivisionFilter.h>
#include <vtkLoopSubdivisionFilter.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTriangleFilter.h>
#include <vtkXMLPolyDataReader.h>

// Readers
#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkXMLPolyDataReader.h>

#include <vtkPolyData.h>
#include <vtkSphereSource.h>

#include <algorithm> // For transform()
#include <cctype>    // For to_lower
#include <string>    // For find_last_of()

namespace {
vtkSmartPointer<vtkPolyData> ReadPolyData(std::string const& fileName);
}

int main(int argc, char* argv[])
{
  // Test with these parameters: footbones.ply 2
  int numberOfSubdivisions = 2;

  auto polyData = ReadPolyData(argc > 1 ? argv[1] : "");
  vtkSmartPointer<vtkPolyData> originalMesh; /* =
      vtkSmartPointer<vtkPolyData>::New();*/
  if (argc > 1) // If a file name is specified, open and use the file.
  {
    // Subdivision filters only work on triangles.
    vtkNew<vtkTriangleFilter> triangles;
    triangles->SetInputData(polyData);
    triangles->Update();
    originalMesh = triangles->GetOutput();
  }
  else
  {
    originalMesh = polyData;
  }
  if (argc > 2)
  {
    numberOfSubdivisions = std::stod(argv[2]);
  }
  std::cout << "Before subdivision" << std::endl;
  std::cout << "    There are " << originalMesh->GetNumberOfPoints()
            << " points." << std::endl;
  std::cout << "    There are " << originalMesh->GetNumberOfPolys()
            << " triangles." << std::endl;

  vtkNew<vtkNamedColors> colors;
  vtkColor3d background = colors->GetColor3d("Gray");
  vtkColor3d originalColor = colors->GetColor3d("Tomato");
  vtkColor3d loopColor = colors->GetColor3d("Banana");
  vtkColor3d butterflyColor = colors->GetColor3d("DodgerBlue");

  double numberOfViewports = 3.0;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetSize(400 * numberOfViewports, 512); //(width, height)
  renderWindow->SetWindowName("SubdivisionDemo");

  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  vtkNew<vtkCamera> camera;

  vtkNew<vtkLegendBoxActor> legend;
  legend->SetNumberOfEntries(3);
  vtkNew<vtkRenderer> legendRen;

  for (unsigned i = 0; i < numberOfViewports; i++)
  {
    // Note: Here we create a superclass pointer (vtkPolyDataAlgorithm) so
    // that we can easily instantiate different types of subdivision filters.
    // Typically you would not want to do this, but rather create the pointer
    // to be the type filter you will actually use, e.g.
    // vtkNew<vtkLinearSubdivisionFilter>  subdivisionFilter;
    vtkSmartPointer<vtkPolyDataAlgorithm> subdivisionFilter;
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkActor> actor;
    switch (i)
    {
    case 0:
      subdivisionFilter = vtkSmartPointer<vtkLinearSubdivisionFilter>::New();
      dynamic_cast<vtkLinearSubdivisionFilter*>(subdivisionFilter.GetPointer())
          ->SetNumberOfSubdivisions(numberOfSubdivisions);
      actor->GetProperty()->SetDiffuseColor(originalColor.GetData());
      legend->SetEntry(i, subdivisionFilter->GetOutput(), "Linear",
                       originalColor.GetData());
      break;
    case 1:
      subdivisionFilter = vtkSmartPointer<vtkLoopSubdivisionFilter>::New();
      dynamic_cast<vtkLoopSubdivisionFilter*>(subdivisionFilter.GetPointer())
          ->SetNumberOfSubdivisions(numberOfSubdivisions);
      actor->GetProperty()->SetDiffuseColor(loopColor.GetData());
      legend->SetEntry(i, subdivisionFilter->GetOutput(), "Loop",
                       loopColor.GetData());
      break;
    case 2:
      subdivisionFilter = vtkSmartPointer<vtkButterflySubdivisionFilter>::New();
      dynamic_cast<vtkButterflySubdivisionFilter*>(
          subdivisionFilter.GetPointer())
          ->SetNumberOfSubdivisions(numberOfSubdivisions);
      actor->GetProperty()->SetDiffuseColor(butterflyColor.GetData());
      legend->SetEntry(i, subdivisionFilter->GetOutput(), "Butterfly",
                       butterflyColor.GetData());
      break;
    default:
      break;
    }
    subdivisionFilter->SetInputData(originalMesh);
    subdivisionFilter->Update();

    renderWindow->AddRenderer(renderer);
    // We reserve the lower part of the viewport for the legend.
    renderer->SetViewport(static_cast<double>(i) / numberOfViewports, 0.2,
                          static_cast<double>(i + 1) / numberOfViewports, 1);
    renderer->SetBackground(background.GetData());
    renderer->SetActiveCamera(camera);
    renderer->UseHiddenLineRemovalOn();
    // Create a mapper and actor
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(subdivisionFilter->GetOutputPort());
    actor->SetMapper(mapper);
    renderer->AddActor(actor);
    renderer->ResetCamera();
  }

  // Position the legend below the three viewports defined above.
  legend->GetPositionCoordinate()->SetCoordinateSystemToView();
  legend->GetPositionCoordinate()->SetValue(-0.3, -0.9);
  legend->GetPosition2Coordinate()->SetCoordinateSystemToView();
  legend->GetPosition2Coordinate()->SetValue(0.3, 0.9);
  legend->BorderOff();
  legendRen->AddActor(legend);
  legendRen->SetBackground(background.GetData());
  legendRen->SetViewport(0.0, 0.0, 1.0, 0.2);
  renderWindow->AddRenderer(legendRen);

  renderWindow->Render();

  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}

namespace {
vtkSmartPointer<vtkPolyData> ReadPolyData(std::string const& fileName)
{
  vtkSmartPointer<vtkPolyData> polyData;
  std::string extension = "";
  if (fileName.find_last_of(".") != std::string::npos)
  {
    extension = fileName.substr(fileName.find_last_of("."));
  }
  // Make the extension lowercase.
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 ::tolower);
  if (extension == ".ply")
  {
    vtkNew<vtkPLYReader> reader;
    reader->SetFileName(fileName.c_str());
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".vtp")
  {
    vtkNew<vtkXMLPolyDataReader> reader;
    reader->SetFileName(fileName.c_str());
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".obj")
  {
    vtkNew<vtkOBJReader> reader;
    reader->SetFileName(fileName.c_str());
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".stl")
  {
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName(fileName.c_str());
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".vtk")
  {
    vtkNew<vtkPolyDataReader> reader;
    reader->SetFileName(fileName.c_str());
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".g")
  {
    vtkNew<vtkBYUReader> reader;
    reader->SetGeometryFileName(fileName.c_str());
    reader->Update();
    polyData = reader->GetOutput();
  }
  else
  {
    // Return a polydata sphere if the extension is unknown.
    vtkNew<vtkSphereSource> source;
    source->SetThetaResolution(20);
    source->SetPhiResolution(11);
    source->Update();
    polyData = source->GetOutput();
  }
  return polyData;
}
} // namespace
