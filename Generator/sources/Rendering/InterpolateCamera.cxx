#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkCameraInterpolator.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkSphereSource.h>
#include <vtkXMLPolyDataReader.h>

#include <vtksys/SystemTools.hxx>

#include <array>
#include <chrono>
#include <iterator>
// #include <random>
#include <thread>
#include <vector>

namespace {
vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName);

void ComputeKeyPoints(vtkPolyData* polyData, std::array<double, 3>& center,
                      std::vector<std::array<double, 3>>& keyPoints);

template <class T, std::size_t N>
ostream& operator<<(ostream& o, const std::array<T, N>& arr)
{
  copy(arr.cbegin(), arr.cend(), std::ostream_iterator<T>(o, ", "));
  return o;
}

template <class T, std::size_t N>
ostream& operator<<(ostream& o, const std::vector<T>& vec)
{
  copy(vec.cbegin(), vec.cend(), std::ostream_iterator<T>(o, ", "));
  return o;
}

} // namespace

int main(int argc, char* argv[])
{

  vtkNew<vtkNamedColors> colors;

  vtkSmartPointer<vtkPolyData> polyData = ReadPolyData(argc > 1 ? argv[1] : "");

  // Setup camera views for interpolation
  vtkNew<vtkCameraInterpolator> interpolator;
  interpolator->SetInterpolationTypeToSpline();

  std::array<double, 3> center;
  std::vector<std::array<double, 3>> keyPoints;
  ComputeKeyPoints(polyData, center, keyPoints);

  for (size_t i = 0; i < keyPoints.size() + 1; ++i)
  {
    // auto j = i;
    vtkNew<vtkCamera> cam;
    cam->SetFocalPoint(center.data());
    if (i < keyPoints.size())
    {
      cam->SetPosition(keyPoints[i].data());
    }
    else
    {
      cam->SetPosition(keyPoints[0].data());
    }
    cam->SetViewUp(0.0, 0.0, 1.0);
    interpolator->AddCamera((double)i, cam);
  }

  // Visualize
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputData(polyData);
  mapper->ScalarVisibilityOff();

  vtkNew<vtkProperty> backProp;
  backProp->SetDiffuseColor(colors->GetColor3d("Banana").GetData());
  backProp->SetDiffuse(.76);
  backProp->SetSpecular(.4);
  backProp->SetSpecularPower(30);
  ;

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->SetBackfaceProperty(backProp);
  actor->GetProperty()->SetDiffuseColor(
      colors->GetColor3d("Crimson").GetData());
  actor->GetProperty()->SetSpecular(.6);
  actor->GetProperty()->SetSpecularPower(30);

  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(640, 480);
  renderWindow->SetWindowName("InterpolateCamera");

  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("Silver").GetData());

  vtkNew<vtkCamera> camera;
  renderer->SetActiveCamera(camera);

  auto numSteps = 600;
  auto minT = interpolator->GetMinimumT();
  auto maxT = interpolator->GetMaximumT();
  for (auto i = 0; i < numSteps; ++i)
  {
    double t = (double)i * (maxT - minT) / (double)(numSteps - 1);
    interpolator->InterpolateCamera(t, camera);
    renderer->ResetCameraClippingRange();
    renderWindow->Render();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}

namespace {
vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName)
{
  vtkSmartPointer<vtkPolyData> polyData;
  std::string extension =
      vtksys::SystemTools::GetFilenameLastExtension(std::string(fileName));
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
  else if (extension == ".vtk")
  {
    vtkNew<vtkPolyDataReader> reader;
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
    vtkNew<vtkSphereSource> source;
    source->Update();
    polyData = source->GetOutput();
  }
  return polyData;
}

void ComputeKeyPoints(vtkPolyData* polyData, std::array<double, 3>& center,
                      std::vector<std::array<double, 3>>& keyPoints)
{
  // std::mt19937 mt(4355412); // Standard mersenne_twister_engine
  // std::uniform_real_distribution<double> dis(1.0, 3.0);

  vtkNew<vtkMinimalStandardRandomSequence> randomSequence;
  randomSequence->SetSeed(4355412);

  // Get Bounding Box
  std::array<double, 6> bounds;
  polyData->GetBounds(bounds.data());

  double range;
  range = std::max(std::max(bounds[1] - bounds[0], bounds[3] - bounds[2]),
                   bounds[5] - bounds[3]);

  std::vector<std::array<double, 3>> points(8);
  std::array<double, 3> point;
  point = {{bounds[0], bounds[2], bounds[4]}};
  points[0] = point;

  point = {{bounds[1], bounds[2], bounds[4]}};
  points[1] = point;

  point = {{bounds[1], bounds[2], bounds[5]}};
  points[2] = point;

  point = {{bounds[0], bounds[2], bounds[5]}};
  points[3] = point;

  point = {{bounds[0], bounds[3], bounds[4]}};
  points[4] = point;

  point = {{bounds[1], bounds[3], bounds[4]}};
  points[5] = point;

  point = {{bounds[1], bounds[3], bounds[5]}};
  points[6] = point;

  point = {{bounds[0], bounds[3], bounds[5]}};
  points[7] = point;

  polyData->GetCenter(center.data());

  for (size_t i = 0; i < points.size(); ++i)
  {
    std::array<double, 3> direction;
    for (auto j = 0; j < 3; ++j)
    {
      direction[j] = points[i][j] - center[j];
    }
    vtkMath::Normalize(direction.data());
    // double factor = dis(mt);
    double factor = randomSequence->GetRangeValue(1.0, 3.0);
    randomSequence->Next();
    keyPoints.resize(8);
    for (auto j = 0; j < 3; ++j)
    {
      keyPoints[i][j] = points[i][j] + direction[j] * range * factor;
    }
  }
}
} // namespace
