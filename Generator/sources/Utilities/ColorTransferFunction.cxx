#include <vtkColorTransferFunction.h>
#include <vtkNew.h>

int main(int, char*[])
{
  vtkNew<vtkColorTransferFunction> colorTransferFunction;

  colorTransferFunction->AddRGBPoint(0.0, 1, 0, 0);
  colorTransferFunction->AddRGBPoint(10.0, 0, 1, 0);

  double color[3];
  colorTransferFunction->GetColor(1.0, color);
  std::cout << color[0] << " " << color[1] << " " << color[2] << std::endl;

  colorTransferFunction->GetColor(5.0, color);
  std::cout << color[0] << " " << color[1] << " " << color[2] << std::endl;

  return EXIT_SUCCESS;
}
