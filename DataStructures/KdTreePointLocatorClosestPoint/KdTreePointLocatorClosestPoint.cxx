#include <vtkKdTreePointLocator.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

int main(int, char*[])
{
  // Setup point coordinates
  double x[3] = {1.0, 0.0, 0.0};
  double y[3] = {0.0, 1.0, 0.0};
  double z[3] = {0.0, 0.0, 1.0};

  vtkNew<vtkPoints> points;
  points->InsertNextPoint(x);
  points->InsertNextPoint(y);
  points->InsertNextPoint(z);

  vtkNew<vtkPolyData> polydata;
  polydata->SetPoints(points);

  // Create the tree
  vtkNew<vtkKdTreePointLocator> kDTree;
  kDTree->SetDataSet(polydata);
  kDTree->BuildLocator();

  double testPoint[3] = {2.0, 0.0, 0.0};

  // Find the closest points to TestPoint
  vtkIdType iD = kDTree->FindClosestPoint(testPoint);
  std::cout << "The closest point is point " << iD << std::endl;

  // Get the coordinates of the closest point
  double closestPoint[3];
  kDTree->GetDataSet()->GetPoint(iD, closestPoint);
  std::cout << "Coordinates: " << closestPoint[0] << " " << closestPoint[1]
            << " " << closestPoint[2] << std::endl;

  return EXIT_SUCCESS;
}
