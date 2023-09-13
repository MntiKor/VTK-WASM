#include <vtkActor.h>
#include <vtkCleanPolyData.h>
#include <vtkExtractEnclosedPoints.h>
#include <vtkLineSource.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOBBTree.h>
#include <vtkPoints.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkQuad.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTubeFilter.h>

// Readers
#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkXMLPolyDataReader.h>

#include <vtkPolyData.h>
#include <vtkSphereSource.h>

#include <array>
#include <cctype> // For to_lower
#include <cstdlib>
#include <iostream>
#include <string> // For find_last_of()

namespace {
vtkSmartPointer<vtkPolyData> ReadPolyData(std::string const& fileName);
double MakeAQuad(std::vector<std::array<double, 3>>&, std::array<double, 3>&);

} // namespace

int main(int argc, char* argv[])
{
  auto polyData = ReadPolyData(argc > 1 ? argv[1] : "");
  ;

  // Get bounds of polydata.
  std::array<double, 6> bounds;
  polyData->GetBounds(bounds.data());

  // Create the tree.
  vtkNew<vtkOBBTree> obbTree;
  obbTree->SetDataSet(polyData);
  obbTree->SetMaxLevel(1);
  obbTree->BuildLocator();

  // Get the PolyData for the OBB.
  vtkNew<vtkPolyData> obbPolydata;
  obbTree->GenerateRepresentation(0, obbPolydata);

  // Get the points of the OBB.
  vtkNew<vtkPoints> obbPoints;
  obbPoints->DeepCopy(obbPolydata->GetPoints());

  // Use a quad to find centers of OBB faces.
  vtkNew<vtkQuad> aQuad;

  std::vector<std::array<double, 3>> facePoints(4);
  std::vector<std::array<double, 3>> centers(3);
  std::vector<std::array<double, 3>> endPoints(3);

  std::array<double, 3> center;
  std::array<double, 3> endPoint;
  std::array<double, 3> point0, point1, point2, point3, point4, point5, point6,
      point7;
  std::array<double, 3> radii;
  std::array<double, 3> lengths;

  // Transfer the points to std::array's.
  obbPoints->GetPoint(0, point0.data());
  obbPoints->GetPoint(1, point1.data());
  obbPoints->GetPoint(2, point2.data());
  obbPoints->GetPoint(3, point3.data());
  obbPoints->GetPoint(4, point4.data());
  obbPoints->GetPoint(5, point5.data());
  obbPoints->GetPoint(6, point6.data());
  obbPoints->GetPoint(7, point7.data());

  // x face.
  // ids[0] = 2; ids[1] = 3; ids[2] = 7; ids[3] = 6;
  facePoints[0] = point2;
  facePoints[1] = point3;
  facePoints[2] = point7;
  facePoints[3] = point6;
  radii[0] = MakeAQuad(facePoints, centers[0]);
  MakeAQuad(facePoints, centers[0]);
  // ids[0] = 0; ids[1] = 4; ids[2] = 5; ids[3] = 1;
  facePoints[0] = point0;
  facePoints[1] = point4;
  facePoints[2] = point5;
  facePoints[3] = point1;
  MakeAQuad(facePoints, endPoints[0]);
  lengths[0] = std::sqrt(vtkMath::Distance2BetweenPoints(centers[0].data(),
                                                         endPoints[0].data())) /
      2.0;

  // y face.
  // ids[0] = 0; ids[1] = 1; ids[2] = 2; ids[3] = 3;
  facePoints[0] = point0;
  facePoints[1] = point1;
  facePoints[2] = point2;
  facePoints[3] = point3;
  radii[1] = MakeAQuad(facePoints, centers[1]);
  // ids[0] = 4; ids[1] = 6; ids[2] = 7; ids[3] = 5;
  facePoints[0] = point4;
  facePoints[1] = point6;
  facePoints[2] = point7;
  facePoints[3] = point5;
  MakeAQuad(facePoints, endPoints[1]);
  lengths[1] = std::sqrt(vtkMath::Distance2BetweenPoints(centers[1].data(),
                                                         endPoints[1].data())) /
      2.0;

  // z face.
  // ids[0] = 0; ids[1] = 2; ids[2] = 6; ids[3] = 4;
  facePoints[0] = point0;
  facePoints[1] = point2;
  facePoints[2] = point6;
  facePoints[3] = point4;
  MakeAQuad(facePoints, centers[2]);
  radii[2] =
      std::sqrt(vtkMath::Distance2BetweenPoints(point0.data(), point2.data())) /
      2.0;
  double outerRadius =
      std::sqrt(vtkMath::Distance2BetweenPoints(point0.data(), point6.data())) /
      2.0;

  // ids[0] = 1; ids[1] = 3; ids[2] = 7; ids[3] = 5;
  facePoints[0] = point1;
  facePoints[1] = point5;
  facePoints[2] = point7;
  facePoints[3] = point3;
  MakeAQuad(facePoints, endPoints[2]);
  lengths[2] = std::sqrt(vtkMath::Distance2BetweenPoints(centers[2].data(),
                                                         endPoints[2].data())) /
      2.0;

  // Find long axis.
  int longAxis = -1;
  double length = VTK_DOUBLE_MIN;
  for (auto i = 0; i < 3; ++i)
  {
    std::cout << "length: " << lengths[i] << std::endl;
    if (lengths[i] > length)
    {
      length = lengths[i];
      longAxis = i;
    }
  }
  std::cout << "longAxis: " << longAxis << std::endl;
  std::cout << "radii: ";
  double radius = radii[longAxis];
  for (auto& a : radii)
  {
    std::cout << a << ", ";
  }
  std::cout << std::endl;
  std::cout << "radius: " << radius << std::endl;
  std::cout << "outerRadius: " << outerRadius << std::endl;
  center = centers[longAxis];
  endPoint = endPoints[longAxis];

  vtkNew<vtkNamedColors> colors;

  vtkNew<vtkLineSource> lineSource;
  lineSource->SetPoint1(center.data());
  lineSource->SetPoint2(endPoint.data());

  vtkNew<vtkTubeFilter> tube;
  tube->SetInputConnection(lineSource->GetOutputPort());
  tube->SetRadius(radius);
  tube->SetNumberOfSides(51);
  tube->CappingOn();
  tube->Update();

  // See if all points lie inside cylinder.
  vtkNew<vtkCleanPolyData> clean;
  clean->SetInputData(tube->GetOutput());
  clean->Update();

  vtkNew<vtkExtractEnclosedPoints> enclosedPoints;
  enclosedPoints->SetSurfaceData(clean->GetOutput());
  enclosedPoints->SetInputData(polyData);
  enclosedPoints->SetTolerance(.0001);
  enclosedPoints->GenerateOutliersOn();
  enclosedPoints->CheckSurfaceOn();
  enclosedPoints->Update();

  std::cout << "polyData points: " << polyData->GetPoints()->GetNumberOfPoints()
            << " excluded points: "
            << enclosedPoints->GetOutput(1)->GetPoints()->GetNumberOfPoints()
            << std::endl;

  vtkNew<vtkPolyDataMapper> repMapper;
  repMapper->SetInputData(obbPolydata);
  vtkNew<vtkActor> repActor;
  repActor->SetMapper(repMapper);
  repActor->GetProperty()->SetColor(colors->GetColor3d("peacock").GetData());
  repActor->GetProperty()->SetOpacity(.6);

  // Create a mapper and actor for the cylinder.
  vtkNew<vtkPolyDataMapper> cylinderMapper;
  cylinderMapper->SetInputConnection(tube->GetOutputPort());

  vtkNew<vtkActor> cylinderActor;

  cylinderActor->SetMapper(cylinderMapper);
  cylinderActor->GetProperty()->SetColor(
      colors->GetColor3d("banana").GetData());
  cylinderActor->GetProperty()->SetOpacity(.5);

  vtkNew<vtkPolyDataMapper> originalMapper;
  originalMapper->SetInputData(polyData);
  vtkNew<vtkActor> originalActor;
  originalActor->SetMapper(originalMapper);
  originalActor->GetProperty()->SetColor(
      colors->GetColor3d("tomato").GetData());

  // Create a renderer, render window, and interactor.
  vtkNew<vtkRenderer> renderer;
  renderer->UseHiddenLineRemovalOn();

  // Display all centers and endpoints.
  std::vector<vtkColor3d> cs;
  cs.push_back(colors->GetColor3d("red"));
  cs.push_back(colors->GetColor3d("green"));
  cs.push_back(colors->GetColor3d("blue"));
  for (auto i = 0; i < 3; ++i)
  {
    vtkNew<vtkSphereSource> ps1;
    ps1->SetCenter(centers[i].data());
    ps1->SetRadius(length * .04);
    ps1->SetPhiResolution(21);
    ps1->SetThetaResolution(41);
    vtkNew<vtkPolyDataMapper> pm1;
    pm1->SetInputConnection(ps1->GetOutputPort());
    vtkNew<vtkActor> pa1;
    pa1->GetProperty()->SetColor(cs[i].GetData());
    pa1->GetProperty()->SetSpecularPower(50);
    pa1->GetProperty()->SetSpecular(.4);
    pa1->GetProperty()->SetDiffuse(.6);
    pa1->SetMapper(pm1);
    renderer->AddActor(pa1);

    vtkNew<vtkSphereSource> ps2;
    ps2->SetCenter(endPoints[i].data());
    ps2->SetRadius(length * .04);
    ps2->SetPhiResolution(21);
    ps2->SetThetaResolution(41);
    vtkNew<vtkPolyDataMapper> pm2;
    pm2->SetInputConnection(ps2->GetOutputPort());
    vtkNew<vtkActor> pa2;
    pa2->GetProperty()->SetColor(cs[i].GetData());
    pa2->SetMapper(pm2);
    renderer->AddActor(pa2);
  }

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("OrientedBoundingCylinder");
  renderWindow->SetSize(640, 480);

  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);
  // Add the actors to the scene.
  renderer->AddActor(originalActor);
  renderer->AddActor(cylinderActor);
  //  renderer->AddActor(repActor);

  renderer->GradientBackgroundOn();
  renderer->SetBackground2(colors->GetColor3d("SkyBlue").GetData());
  renderer->SetBackground(colors->GetColor3d("LightSeaGreen").GetData());

  // double adjustedRadius = radius;
  double adjustedIncr = (outerRadius - radius) / 20.0;
  if (enclosedPoints->GetOutput(1)->GetPoints()->GetNumberOfPoints() > 4)
  {
    std::cout << "improving..." << std::endl;
    for (double r = radius;
         enclosedPoints->GetOutput(1)->GetPoints()->GetNumberOfPoints() > 4;
         r += adjustedIncr)
    {
      tube->SetRadius(r);
      tube->Update();
      clean->Update();
      enclosedPoints->Update();
      if (enclosedPoints->GetOutput(1)->GetPoints() != nullptr)
      {
        std::cout << "r: " << r << std::endl;
        std::cout
            << " excluded points: "
            << enclosedPoints->GetOutput(1)->GetPoints()->GetNumberOfPoints()
            << std::endl;
        renderWindow->Render();
      }
      else
      {
        break;
      }
    }
  }

  // Render and interact.
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
double MakeAQuad(std::vector<std::array<double, 3>>& points,
                 std::array<double, 3>& center)
{
  vtkNew<vtkQuad> aQuad;
  aQuad->GetPoints()->SetPoint(0, points[0].data());
  aQuad->GetPoints()->SetPoint(1, points[1].data());
  aQuad->GetPoints()->SetPoint(2, points[2].data());
  aQuad->GetPoints()->SetPoint(3, points[3].data());
  aQuad->GetPointIds()->SetId(0, 0);
  aQuad->GetPointIds()->SetId(1, 1);
  aQuad->GetPointIds()->SetId(2, 2);
  aQuad->GetPointIds()->SetId(3, 3);

  std::array<double, 3> pcenter;
  pcenter[0] = pcenter[1] = pcenter[2] = -12345.0;
  aQuad->GetParametricCenter(pcenter.data());
  std::vector<double> cweights(aQuad->GetNumberOfPoints());
  int pSubId = 0;
  aQuad->EvaluateLocation(pSubId, pcenter.data(), center.data(),
                          &(*cweights.begin()));

  std::cout << "center: ";
  for (auto& a : center)
  {
    std::cout << a << ", ";
  }
  std::cout << std::endl;

  return std::sqrt(aQuad->GetLength2()) / 2.0;
}
} // namespace
