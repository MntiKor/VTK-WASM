#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkSphereSource.h>

int main(int, char*[])
{
  // Create a sphere
  vtkNew<vtkSphereSource> sphereSource;
  sphereSource->Update();

  vtkPolyData* polydata = sphereSource->GetOutput();

  // Write all of the coordinates of the points in the vtkPolyData to the
  // console.
  for (vtkIdType i = 0; i < polydata->GetNumberOfPoints(); i++)
  {
    double p[3];
    polydata->GetPoint(i, p);
    // This is identical to:
    // polydata->GetPoints()->GetPoint(i,p);
    std::cout << "Point " << i << " : (" << p[0] << " " << p[1] << " " << p[2]
              << ")" << std::endl;
  }

  return EXIT_SUCCESS;
}
