#include <vtkDenseArray.h>
#include <vtkNew.h>

#include <iostream>
#include <string>

int main(int, char*[])
{
  vtkNew<vtkDenseArray<double>> array;
  array->Resize(5, 1);

  for (unsigned int i = 0; i < 5; i++)
  {
    array->SetValue(i, 0, 2.0 + i);
    double val = array->GetValue(i, 0);
    std::cout << "val: " << val << std::endl;
  }

  // int extent = array->GetExtents()[1];
  int dim1start = array->GetExtent(0).GetBegin();
  int dim1end = array->GetExtent(0).GetEnd();
  std::cout << "dim1: " << dim1start << " to " << dim1end << std::endl;
  // int dim2 = array->GetExtent(1)[1];

  return EXIT_SUCCESS;
}
