#include <vtkNew.h>
#include <vtkPolyData.h>

int main(int, char*[])
{
  vtkNew<vtkPolyData> polydata;

  vtkNew<vtkPolyData> polydataCopy;

  polydataCopy->ShallowCopy(polydata);

  return EXIT_SUCCESS;
}
