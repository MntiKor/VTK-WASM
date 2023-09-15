#include <vtkAxis.h>
#include <vtkChartXY.h>
#include <vtkContextView.h>
#include <vtkFloatArray.h>
#include <vtkKdTreePointLocator.h>
#include <vtkMinimalStandardRandomSequence.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPlot.h>
#include <vtkPolyData.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkTable.h>
#include <vtkTimerLog.h>

#include <time.h>
#include <vector>

namespace {
void RandomPointInBounds(vtkPolyData* polydata, double p[3],
                         vtkMinimalStandardRandomSequence* rng);

double TimeKDTree(vtkPolyData* polydata, int maxPoints, int numberOfTrials,
                  vtkMinimalStandardRandomSequence* rng);
} // namespace

int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;

  vtkNew<vtkSphereSource> reader;
  reader->SetThetaResolution(30);
  reader->SetPhiResolution(30);
  reader->Update();

  vtkNew<vtkMinimalStandardRandomSequence> rng;
  rng->SetSeed(8775070);
  // rng->SetSeed(0);

  std::cout << "Timing KD tree..." << std::endl;
  std::vector<std::pair<int, double>> results;
  int numberOfTrials = 1000;
  for (int i = 1; i < 20; i++)
  {
    double t = TimeKDTree(reader->GetOutput(), i, numberOfTrials, rng);
    std::pair<int, double> result(i, t);
    results.push_back(result);
  }

  // Create a table with some points in it
  vtkNew<vtkTable> table;

  vtkNew<vtkFloatArray> maxPointsPerRegion;
  maxPointsPerRegion->SetName("MaxPointsPerRegion");
  table->AddColumn(maxPointsPerRegion);

  vtkNew<vtkFloatArray> runtime;
  runtime->SetName("Run time");
  table->AddColumn(runtime);

  // Fill in the table with some example values
  size_t numPoints = results.size();
  table->SetNumberOfRows(static_cast<vtkIdType>(numPoints));
  for (size_t i = 0; i < numPoints; ++i)
  {
    table->SetValue(static_cast<vtkIdType>(i), 0, results[i].first);
    table->SetValue(static_cast<vtkIdType>(i), 1, results[i].second);
    std::cout << "Put " << results[i].first << " " << results[i].second
              << " in the table." << std::endl;
  }

  // Set up the view
  vtkNew<vtkContextView> view;
  view->GetRenderer()->SetBackground(colors->GetColor3d("White").GetData());
  view->GetRenderWindow()->SetWindowName("KDTreeTimingDemo");

  // Add multiple line plots, setting the colors etc
  vtkNew<vtkChartXY> chart;
  view->GetScene()->AddItem(chart);
  vtkPlot* line = chart->AddPlot(vtkChart::LINE);
  line->SetInputData(table, 0, 1);
  auto lineColor = colors->HTMLColorToRGBA("Lime").GetData();
  line->SetColor(lineColor[0], lineColor[1], lineColor[2], lineColor[3]);
  line->SetWidth(3.0);
  line->GetXAxis()->SetTitle("Max Points Per Region");
  line->GetYAxis()->SetTitle("Run time");

  view->GetRenderWindow()->SetMultiSamples(0);
  view->GetRenderWindow()->Render();

  // Start interactor
  view->GetInteractor()->Initialize();
  view->GetInteractor()->Start();

  return EXIT_SUCCESS;
}

namespace {
void RandomPointInBounds(vtkPolyData* polydata, double p[3],
                         vtkMinimalStandardRandomSequence* rng)
{
  double bounds[6];
  polydata->GetBounds(bounds);

  for (auto i = 0; i < 3; ++i)
  {
    p[i] = bounds[i * 2] +
        (bounds[i * 2 + 1] - bounds[i * 2]) * rng->GetRangeValue(0.0, 1.0);
    rng->Next();
  }
}

double TimeKDTree(vtkPolyData* polydata, int maxLevel, int numberOfTrials,
                  vtkMinimalStandardRandomSequence* rng)
{
  vtkNew<vtkTimerLog> timer;
  timer->StartTimer();

  // Create the tree
  vtkNew<vtkKdTreePointLocator> kdtree;
  kdtree->SetDataSet(polydata);
  kdtree->AutomaticOff();
  kdtree->SetMaxLevel(maxLevel);
  kdtree->BuildLocator();

  for (int i = 0; i < numberOfTrials; i++)
  {
    double p[3];
    RandomPointInBounds(polydata, p, rng);
    kdtree->FindClosestPoint(p);
  }

  timer->StopTimer();

  std::cout << "KDTree took " << timer->GetElapsedTime() << std::endl;

  return timer->GetElapsedTime();
}
} // namespace
