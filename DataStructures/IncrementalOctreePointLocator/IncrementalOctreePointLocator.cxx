#include <vtkIncrementalOctreePointLocator.h>
#include <vtkMath.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

#include <cmath>

int main(int, char*[])
{
  // Setup point coordinates.
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
  vtkNew<vtkIncrementalOctreePointLocator> octree;
  octree->SetDataSet(polydata);
  octree->BuildLocator();

  auto pointCoordinates = [](double* pt) {
    std::cout << "Coordinates: " << pt[0] << " " << pt[1] << " " << pt[2]
              << std::endl;
  };
  double testPoint[3] = {2.0, 0.0, 0.0};
  std::cout << "Test Point ";
  pointCoordinates(testPoint);

  auto closestPoint = [&octree, pointCoordinates](double* testPoint) {
    // Find the closest points to TestPoint.
    vtkIdType id = octree->FindClosestPoint(testPoint);
    std::cout << "The closest point is point " << id << std::endl;

    // Get the coordinates of the closest point.
    double pt[3];
    octree->GetDataSet()->GetPoint(id, pt);
    pointCoordinates(pt);
    std::cout << "Distance: "
              << std::sqrt(vtkMath::Distance2BetweenPoints(testPoint, pt))
              << std::endl;
  };

  closestPoint(testPoint);

  // Insert another point.
  double pnew[3] = {2.1, 0, 0};
  octree->InsertNextPoint(pnew);

  closestPoint(testPoint);

  return EXIT_SUCCESS;
}
