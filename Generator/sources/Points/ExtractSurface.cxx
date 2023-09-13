#include <vtkCamera.h>
#include <vtkExtractSurface.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPCANormalEstimation.h>
#include <vtkPointData.h>
#include <vtkPointSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSignedDistance.h>
#include <vtkSmartPointer.h>

#include <vtksys/SystemTools.hxx>

#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkXMLPolyDataReader.h>

namespace {
vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName);
}

int main(int argc, char* argv[])
{
  auto polyData = ReadPolyData(argc > 1 ? argv[1] : "");

  std::cout << "# of points: " << polyData->GetNumberOfPoints() << std::endl;

  double bounds[6];
  polyData->GetBounds(bounds);
  double range[3];
  for (int i = 0; i < 3; ++i)
  {
    range[i] = bounds[2 * i + 1] - bounds[2 * i];
  }

  int sampleSize = polyData->GetNumberOfPoints() * 0.00005;
  if (sampleSize < 10)
  {
    sampleSize = 10;
  }
  std::cout << "Sample size is: " << sampleSize << std::endl;
  // Do we need to estimate normals?
  vtkNew<vtkSignedDistance> distance;
  if (polyData->GetPointData()->GetNormals())
  {
    std::cout << "Using normals from input file" << std::endl;
    distance->SetInputData(polyData);
  }
  else
  {
    std::cout << "Estimating normals using PCANormalEstimation" << std::endl;
    vtkNew<vtkPCANormalEstimation> normals;
    normals->SetInputData(polyData);
    normals->SetSampleSize(sampleSize);
    normals->SetNormalOrientationToGraphTraversal();
    normals->FlipNormalsOn();
    distance->SetInputConnection(normals->GetOutputPort());
  }
  std::cout << "Range: " << range[0] << ", " << range[1] << ", " << range[2]
            << std::endl;
  int dimension = 256;
  double radius;
  radius = std::max(std::max(range[0], range[1]), range[2]) /
      static_cast<double>(dimension) * 4; // ~4 voxels
  std::cout << "Radius: " << radius << std::endl;

  distance->SetRadius(radius);
  distance->SetDimensions(dimension, dimension, dimension);
  distance->SetBounds(bounds[0] - range[0] * .1, bounds[1] + range[0] * .1,
                      bounds[2] - range[1] * .1, bounds[3] + range[1] * .1,
                      bounds[4] - range[2] * .1, bounds[5] + range[2] * .1);

  vtkNew<vtkExtractSurface> surface;
  surface->SetInputConnection(distance->GetOutputPort());
  surface->SetRadius(radius * .99);
  surface->Update();

  vtkNew<vtkPolyDataMapper> surfaceMapper;
  surfaceMapper->SetInputConnection(surface->GetOutputPort());

  vtkNew<vtkNamedColors> colors;

  vtkNew<vtkProperty> back;
  back->SetColor(colors->GetColor3d("Banana").GetData());

  vtkNew<vtkActor> surfaceActor;
  surfaceActor->SetMapper(surfaceMapper);
  surfaceActor->GetProperty()->SetColor(colors->GetColor3d("Tomato").GetData());
  surfaceActor->SetBackfaceProperty(back);

  // Create graphics stuff.
  vtkNew<vtkRenderer> ren1;
  ren1->SetBackground(colors->GetColor3d("SlateGray").GetData());

  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(ren1);
  renWin->SetSize(512, 512);
  renWin->SetWindowName("ExtractSurface");

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);

  // Add the actors to the renderer,set the background and size.
  ren1->AddActor(surfaceActor);

  // Generate an interesting view.
  ren1->ResetCamera();
  ren1->GetActiveCamera()->Azimuth(120);
  ren1->GetActiveCamera()->Elevation(30);
  ren1->GetActiveCamera()->Dolly(1.0);
  ren1->ResetCameraClippingRange();

  renWin->Render();
  iren->Initialize();
  iren->Start();

  return EXIT_SUCCESS;
}

namespace {
vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName)
{
  vtkSmartPointer<vtkPolyData> polyData;
  std::string extension =
      vtksys::SystemTools::GetFilenameExtension(std::string(fileName));
  if (extension == ".ply")
  {
    vtkNew<vtkPLYReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".vtp")
  {
    vtkNew<vtkXMLPolyDataReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".vtk")
  {
    vtkNew<vtkPolyDataReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".obj")
  {
    vtkNew<vtkOBJReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".stl")
  {
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".g")
  {
    vtkNew<vtkBYUReader> reader;
    reader->SetGeometryFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else
  {
    vtkNew<vtkMinimalStandardRandomSequence> randomSequence;
    randomSequence->SetSeed(8775070);

    vtkNew<vtkPointSource> points;
    points->SetNumberOfPoints(1000);
    points->SetRadius(1.0);
    double x, y, z;
    // random position
    x = randomSequence->GetRangeValue(-1.0, 1.0);
    randomSequence->Next();
    y = randomSequence->GetRangeValue(-1.0, 1.0);
    randomSequence->Next();
    z = randomSequence->GetRangeValue(-1.0, 1.0);
    randomSequence->Next();
    points->SetCenter(x, y, z);
    points->SetDistributionToShell();
    points->Update();
    polyData = points->GetOutput();
  }
  return polyData;
}
} // namespace
