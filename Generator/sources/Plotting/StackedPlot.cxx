#include <vtkAxis.h>
#include <vtkChartXY.h>
#include <vtkColorSeries.h>
#include <vtkContextView.h>
#include <vtkDoubleArray.h>
#include <vtkIntArray.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPlotStacked.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>

namespace {
// Monthly checkout data.
std::string month_labels[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                              "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
constexpr int book[] = {5675, 5902, 6388, 5990, 5575, 7393,
                        9878, 8082, 6417, 5946, 5526, 5166};
constexpr int new_popular[] = {701, 687, 736, 696, 750, 814,
                               923, 860, 786, 735, 680, 741};
constexpr int periodical[] = {184, 176, 166, 131, 171, 191,
                              231, 166, 197, 162, 152, 143};
constexpr int audiobook[] = {903,  1038, 987,  1073, 1144, 1203,
                             1173, 1196, 1213, 1076, 926,  874};
constexpr int video[] = {1524, 1565, 1627, 1445, 1179, 1816,
                         2293, 1811, 1588, 1561, 1542, 1563};
} // namespace

//----------------------------------------------------------------------------
int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;

  // Set up a 2D scene, add an XY chart to it.
  vtkNew<vtkContextView> view;
  view->GetRenderer()->SetBackground(1.0, 1.0, 1.0);
  view->GetRenderWindow()->SetSize(400, 300);
  vtkNew<vtkChartXY> chart;
  view->GetScene()->AddItem(chart);

  // Create a table with some points in it...
  vtkNew<vtkTable> table;

  vtkNew<vtkStringArray> arrMonthLabel;
  arrMonthLabel->SetNumberOfValues(12);

  vtkNew<vtkDoubleArray> arrXTickPositions;
  arrXTickPositions->SetNumberOfValues(12);

  vtkNew<vtkIntArray> arrMonth;
  arrMonth->SetName("Month");
  table->AddColumn(arrMonth);

  vtkNew<vtkIntArray> arrBook;
  arrBook->SetName("Books");
  table->AddColumn(arrBook);

  vtkNew<vtkIntArray> arrNewPopularBook;
  arrNewPopularBook->SetName("New / Popular");
  table->AddColumn(arrNewPopularBook);

  vtkNew<vtkIntArray> arrPeriodical;
  arrPeriodical->SetName("Periodical");
  table->AddColumn(arrPeriodical);

  vtkNew<vtkIntArray> arrAudiobook;
  arrAudiobook->SetName("Audiobook");
  table->AddColumn(arrAudiobook);

  vtkNew<vtkIntArray> arrVideo;
  arrVideo->SetName("Video");
  table->AddColumn(arrVideo);

  table->SetNumberOfRows(12);
  for (int i = 0; i < 12; i++)
  {
    arrMonthLabel->SetValue(i, month_labels[i]);
    arrXTickPositions->SetValue(i, i);

    arrBook->SetValue(i, book[i]);
    arrNewPopularBook->SetValue(i, new_popular[i]);
    arrPeriodical->SetValue(i, periodical[i]);
    arrAudiobook->SetValue(i, audiobook[i]);
    arrVideo->SetValue(i, video[i]);
  }

  // Set the Month Labels
  chart->GetAxis(1)->SetCustomTickPositions(arrXTickPositions, arrMonthLabel);
  chart->GetAxis(1)->SetMaximum(11);

  chart->GetAxis(1)->SetRange(0, 11);
  chart->GetAxis(1)->SetBehavior(vtkAxis::FIXED);
  chart->GetAxis(1)->SetTitle("Month");

  chart->SetShowLegend(true);

  chart->GetAxis(0)->SetTitle("Checkouts");

  // Add multiple line plots, setting the colors etc.
  vtkPlotStacked* stack = 0;

  // Books
  stack = dynamic_cast<vtkPlotStacked*>(chart->AddPlot(vtkChart::STACKED));
  stack->SetUseIndexForXSeries(true);
  stack->SetInputData(table);
  stack->SetInputArray(1, "Books");
  stack->SetInputArray(2, "New / Popular");
  stack->SetInputArray(3, "Periodical");
  stack->SetInputArray(4, "Audiobook");
  stack->SetInputArray(5, "Video");

  vtkNew<vtkColorSeries> colorSeries;
  colorSeries->SetColorScheme(vtkColorSeries::WILD_FLOWER);
  stack->SetColorSeries(colorSeries);

  // Finally render the scene and compare the image to a reference image.
  view->GetRenderWindow()->SetMultiSamples(0);
  view->GetRenderer()->SetBackground(colors->GetColor3d("Cornsilk").GetData());
  view->GetRenderWindow()->SetSize(600, 400);
  view->GetRenderWindow()->SetWindowName("StackedPlot");
  view->GetRenderWindow()->Render();
  view->GetInteractor()->Initialize();
  view->GetInteractor()->Start();

  return EXIT_SUCCESS;
}
