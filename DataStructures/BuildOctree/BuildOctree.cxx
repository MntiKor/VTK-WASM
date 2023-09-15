#include <vtkNew.h>
#include <vtkOctreePointLocator.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>

int main(int, char*[])
{
  // Setup point coordinates
  double x[3]{1.0, 0.0, 0.0};
  double y[3]{0.0, 1.0, 0.0};
  double z[3]{0.0, 0.0, 1.0};

  vtkNew<vtkPoints> points;

  for (unsigned int i = 0; i < 3; ++i)
  {
    points->InsertNextPoint(x[i], y[i], z[i]);
  }

  std::cout << "There are " << points->GetNumberOfPoints() << " points."
            << std::endl;

  vtkNew<vtkPolyData> polydata;
  polydata->SetPoints(points);

  // Create the tree
  vtkNew<vtkOctreePointLocator> octree;
  octree->SetDataSet(polydata);
  octree->BuildLocator();

  std::cout << "Number of points in tree: "
            << octree->GetDataSet()->GetNumberOfPoints() << std::endl;
  for (vtkIdType i = 0; i < points->GetNumberOfPoints(); i++)
  {
    double p[3];
    octree->GetDataSet()->GetPoint(i, p);
    std::cout << "Point Id: " << i << ": Point: (" << p[0] << ", " << p[1]
              << ", " << p[2] << ")" << std::endl;
  }

  return EXIT_SUCCESS;
}
