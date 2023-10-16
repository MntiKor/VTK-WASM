// Based on:
//  https://gitlab.kitware.com/vtk/vtk/-/blob/master/Rendering/Core/Testing/Cxx/TestGradientBackground.cxx?ref_type=heads
// See:
//  [New in VTK 9.3: Radial Gradient
//  Background](https://www.kitware.com/new-in-vtk-9-3-radial-gradient-background/)

#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkColor.h>
#include <vtkConeSource.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>
#include <vtkViewport.h>
#include <vtk_cli11.h>

// Readers
#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkXMLPolyDataReader.h>

#include <vtkPolyDataMapper2D.h>
#include <vtkPolyLine.h>
#include <vtkProperty2D.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace {

/**
 * @brief ReadPolyData Read from a file containing vtkPolyData.
 *
 * ReadPolyData
 *    If the path is empty a cone is returned.
 *    If the extension is unknown a sphere is returned.
 *
 * @param path - The std::filesystem path to the file.
 * @return The vtkPolyData.
 */
vtkNew<vtkPolyData> ReadPolyData(fs::path const& path);

/**
 * @brief ViewportBorder Set a border around a viewport.
 *
 * @param renderer - The renderer corresponding to the viewport.
 * @param sides - A boolean array corresponding to [top, left, bottom, right].
 * @param borderColor - The color of the border.
 * @param borderWidth - The width of the border.
 */
void ViewportBorder(vtkRenderer* renderer, const std::array<bool, 4>& sides,
                    std::string const& borderColor,
                    double const& borderWidth = 2);

} // namespace

int main(int argc, char* argv[])
{

  CLI::App app{"Demonstrates the background shading options."};

  // Define options
  std::string fileName{""};
  app.add_option("fileName", fileName,
                 "A optional path to a file that contains vtkPolyData e.g. "
                 "star-wars-vader-tie-fighter.obj");
  CLI11_PARSE(app, argc, argv);

  auto path = fs::path(fileName);
  if (!path.empty())
  {
    if (!fs::is_regular_file(path))
    {
      std::cerr << "Unable to find: " << path << std::endl;
      return EXIT_FAILURE;
    }
  }

  auto pd = ReadPolyData(path);

  vtkNew<vtkRenderWindow> renWin;
  renWin->SetWindowName("GradientBackground");

  vtkNew<vtkRenderWindowInteractor> iRen;
  vtkNew<vtkRenderer> renderers[4];

  // For each gradient specify the mode.
  vtkViewport::GradientModes modes[4] = {
      vtkViewport::GradientModes::VTK_GRADIENT_VERTICAL,
      vtkViewport::GradientModes::VTK_GRADIENT_HORIZONTAL,
      vtkViewport::GradientModes::VTK_GRADIENT_RADIAL_VIEWPORT_FARTHEST_SIDE,
      vtkViewport::GradientModes::VTK_GRADIENT_RADIAL_VIEWPORT_FARTHEST_CORNER,
  };

  vtkNew<vtkNamedColors> colors;

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputData(pd);

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(colors->GetColor3d("Honeydew").GetData());
  actor->GetProperty()->SetSpecular(0.3);
  actor->GetProperty()->SetSpecularPower(60.0);

  const auto renWidth = 640;
  const auto renHeight = 480;

  // The bounds for each view port.
  double xmins[4] = {0.0, 0.5, 0.0, 0.5};
  double ymins[4] = {0.0, 0.0, 0.5, 0.5};
  double xmaxs[4] = {0.5, 1.0, 0.5, 1.0};
  double ymaxs[4] = {0.5, 0.5, 1.0, 1.0};

  // Here we select and name the colors.
  // Feel free to change colors.
  const auto bottomColor = colors->GetColor3d("Gold");
  const auto topColor = colors->GetColor3d("OrangeRed");
  const auto leftColor = colors->GetColor3d("Gold");
  const auto rightColor = colors->GetColor3d("OrangeRed");
  const auto centerColor = colors->GetColor3d("Gold");
  const auto sideColor = colors->GetColor3d("OrangeRed");
  const auto cornerColor = colors->GetColor3d("OrangeRed");

  const std::array<std::string, 4> viewportTitle{"Vertical", "Horizontal",
                                                 "Radial Farthest Side",
                                                 "Radial Farthest Corner"};

  // Create one text property for all.
  vtkNew<vtkTextProperty> textProperty;
  textProperty->SetJustificationToCentered();
  textProperty->SetFontSize(renHeight / 12);
  textProperty->SetColor(colors->GetColor3d("MidnightBlue").GetData());

  vtkNew<vtkTextMapper> textMappers[4];
  vtkNew<vtkActor2D> textActors[4];

  // Define borders for the viewports {top, left, bottom, right}.
  std::array<bool, 4> lb{false, true, true, false};
  std::array<bool, 4> lbr{false, true, true, true};
  std::array<bool, 4> tlb{true, true, true, false};
  std::array<bool, 4> tlbr{true, true, true, true};
  std::string borderColor = "DarkGreen";
  auto borderWidth = 4.0;

  for (int i = 0; i < 4; ++i)
  {
    auto textMapper = textMappers[i].Get();
    textMapper->SetInput(viewportTitle[i].c_str());
    textMapper->SetTextProperty(textProperty);

    auto textActor = textActors[i].Get();
    textActor->SetMapper(textMapper);
    textActor->SetPosition(renWidth / 2, 8);

    auto ren = renderers[i].Get();
    ren->AddActor(textActor);
    ren->AddActor(actor);
    ren->GradientBackgroundOn();
    ren->SetGradientMode(modes[i]);

    ren->SetViewport(xmins[i], ymins[i], xmaxs[i], ymaxs[i]);

    switch (i)
    {
    case 0:
    default:
      // Vertical
      ren->SetBackground(bottomColor.GetData());
      ren->SetBackground2(topColor.GetData());
      ViewportBorder(ren, lb, borderColor, borderWidth);
      break;
    case 1:
      // Horizontal
      ren->SetBackground(leftColor.GetData());
      ren->SetBackground2(rightColor.GetData());
      ViewportBorder(ren, lbr, borderColor, borderWidth);
      break;
    case 2:
      // Radial Farthest Side
      ren->SetBackground(centerColor.GetData());
      ren->SetBackground2(sideColor.GetData());
      ViewportBorder(ren, tlb, borderColor, borderWidth);
      break;
    case 3:
      // Radial Farthest Corner
      ren->SetBackground(centerColor.GetData());
      ren->SetBackground2(cornerColor.GetData());
      ViewportBorder(ren, tlbr, borderColor, borderWidth);
      break;
    }

    renWin->AddRenderer(ren);
  }

  renWin->SetInteractor(iRen);
  renWin->Render();

  vtkNew<vtkInteractorStyleTrackballCamera> style;
  iRen->SetInteractorStyle(style);

  iRen->Initialize();
  iRen->UpdateSize(renWidth * 2, renHeight * 2);

  iRen->Start();

  return EXIT_SUCCESS;
}

namespace {

vtkNew<vtkPolyData> ReadPolyData(fs::path const& path)
{

  vtkNew<vtkPolyData> polyData;

  if (path.empty())
  {
    // Default to a cone if the path is empty.
    vtkNew<vtkConeSource> source;
    source->SetResolution(25);
    source->SetDirection(0, 1, 0);
    source->SetHeight(1);
    source->Update();
    polyData->DeepCopy(source->GetOutput());
    return polyData;
  }

  std::string extension = path.extension().generic_string();
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 [](char c) { return std::tolower(c); });

  if (extension == ".ply")
  {
    vtkNew<vtkPLYReader> reader;
    reader->SetFileName(path.generic_string().c_str());
    reader->Update();
    polyData->DeepCopy(reader->GetOutput());
  }
  else if (extension == ".vtp")
  {
    vtkNew<vtkXMLPolyDataReader> reader;
    reader->SetFileName(path.generic_string().c_str());
    reader->Update();
    polyData->DeepCopy(reader->GetOutput());
  }
  else if (extension == ".obj")
  {
    vtkNew<vtkOBJReader> reader;
    reader->SetFileName(path.generic_string().c_str());
    reader->Update();
    polyData->DeepCopy(reader->GetOutput());
  }
  else if (extension == ".stl")
  {
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName(path.generic_string().c_str());
    reader->Update();
    polyData->DeepCopy(reader->GetOutput());
  }
  else if (extension == ".vtk")
  {
    vtkNew<vtkPolyDataReader> reader;
    reader->SetFileName(path.generic_string().c_str());
    reader->Update();
    polyData->DeepCopy(reader->GetOutput());
  }
  else if (extension == ".g")
  {
    vtkNew<vtkBYUReader> reader;
    reader->SetGeometryFileName(path.generic_string().c_str());
    reader->Update();
    polyData->DeepCopy(reader->GetOutput());
  }
  else
  {
    std::cerr << "Warning: " << path
              << " unknown extension, using a sphere instead." << std::endl;
    vtkNew<vtkSphereSource> source;
    source->SetPhiResolution(50);
    source->SetThetaResolution(50);
    source->Update();
    polyData->DeepCopy(source->GetOutput());
  }
  return polyData;
}

void ViewportBorder(vtkRenderer* renderer, std::array<bool, 4> const& sides,
                    std::string const& borderColor, double const& borderWidth)
{
  vtkNew<vtkNamedColors> colors;

  // Points start at upper right and proceed anti-clockwise.
  vtkNew<vtkPoints> points;
  points->SetNumberOfPoints(4);
  points->InsertPoint(0, 1, 1, 0);
  points->InsertPoint(1, 0, 1, 0);
  points->InsertPoint(2, 0, 0, 0);
  points->InsertPoint(3, 1, 0, 0);

  vtkNew<vtkCellArray> cells;
  cells->Initialize();

  if (sides[0])
  {
    // Top
    vtkNew<vtkPolyLine> top;
    top->GetPointIds()->SetNumberOfIds(2);
    top->GetPointIds()->SetId(0, 0);
    top->GetPointIds()->SetId(1, 1);
    cells->InsertNextCell(top);
  }
  if (sides[1])
  {
    // Left
    vtkNew<vtkPolyLine> left;
    left->GetPointIds()->SetNumberOfIds(2);
    left->GetPointIds()->SetId(0, 1);
    left->GetPointIds()->SetId(1, 2);
    cells->InsertNextCell(left);
  }
  if (sides[2])
  {
    // Bottom
    vtkNew<vtkPolyLine> bottom;
    bottom->GetPointIds()->SetNumberOfIds(2);
    bottom->GetPointIds()->SetId(0, 2);
    bottom->GetPointIds()->SetId(1, 3);
    cells->InsertNextCell(bottom);
  }
  if (sides[3])
  {
    // Right
    vtkNew<vtkPolyLine> right;
    right->GetPointIds()->SetNumberOfIds(2);
    right->GetPointIds()->SetId(0, 3);
    right->GetPointIds()->SetId(1, 0);
    cells->InsertNextCell(right);
  }

  // Now make the polydata and display it.
  vtkNew<vtkPolyData> poly;
  poly->Initialize();
  poly->SetPoints(points);
  poly->SetLines(cells);

  // Use normalized viewport coordinates since
  // they are independent of window size.
  vtkNew<vtkCoordinate> coordinate;
  coordinate->SetCoordinateSystemToNormalizedViewport();

  vtkNew<vtkPolyDataMapper2D> mapper;
  mapper->SetInputData(poly);
  mapper->SetTransformCoordinate(coordinate);

  vtkNew<vtkActor2D> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(colors->GetColor3d(borderColor).GetData());

  // Line width should be at least 2 to be visible at extremes.
  actor->GetProperty()->SetLineWidth(borderWidth);
  renderer->AddViewProp(actor);
}

} // namespace
