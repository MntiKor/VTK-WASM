#include <vtkDenseArray.h>
#include <vtkNew.h>

struct Point
{
  double x, y;

  operator vtkVariant() const
  {
    return vtkVariant();
  }
};

int main(int, char*[])
{
  Point point;
  point.x = 1.0;
  point.y = 2.0;

  std::cout << point.x << " " << point.y << std::endl;

  vtkNew<vtkDenseArray<Point>> array;
  array->Resize(5, 5);

  array->SetValue(4, 4, point);

  Point retrievedPoint = array->GetValue(4, 4);
  std::cout << retrievedPoint.x << " " << retrievedPoint.y << std::endl;

  return EXIT_SUCCESS;
}
