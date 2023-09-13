#include <vtkDataSetCollection.h>
#include <vtkKdTree.h>
#include <vtkNew.h>
#include <vtkPoints.h>

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

  // Create the tree
  vtkNew<vtkKdTree> kDTree;
  kDTree->BuildLocatorFromPoints(points);

  double testPoint[3] = {2.0, 0.0, 0.0};

  auto pointCoordinates = [](double* pt) {
    std::cout << "Coordinates: " << pt[0] << " " << pt[1] << " " << pt[2]
              << std::endl;
  };

  // Find the closest point to TestPoint.
  double closestPointDist;
  vtkIdType id = kDTree->FindClosestPoint(
      testPoint, closestPointDist); // vtkKdTree::FindClosestPoint: must build
                                    // locator first
  std::cout << "Test Point ";
  pointCoordinates(testPoint);
  std::cout << "The closest point is point " << id << std::endl;
  // Get the closest point in the KD Tree from the point data.
  std::cout << "Closest point ";
  pointCoordinates(points->GetPoint(id));
  std::cout << "Distance: " << closestPointDist << std::endl;

  return EXIT_SUCCESS;
}
