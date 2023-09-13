#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPerspectiveTransform.h>
#include <vtkTransform.h>

int main(int, char*[])
{
  vtkNew<vtkMatrix4x4> m;
  m->SetElement(0, 0, 1);
  m->SetElement(0, 1, 2);
  m->SetElement(0, 2, 3);
  m->SetElement(0, 3, 4);
  m->SetElement(1, 0, 2);
  m->SetElement(1, 1, 2);
  m->SetElement(1, 2, 3);
  m->SetElement(1, 3, 4);
  m->SetElement(2, 0, 3);
  m->SetElement(2, 1, 2);
  m->SetElement(2, 2, 3);
  m->SetElement(2, 3, 4);
  m->SetElement(3, 0, 4);
  m->SetElement(3, 1, 2);
  m->SetElement(3, 2, 3);
  m->SetElement(3, 3, 4);

  vtkNew<vtkPerspectiveTransform> perspectiveTransform;
  perspectiveTransform->SetMatrix(m);

  vtkNew<vtkTransform> transform;
  transform->SetMatrix(m);

  double p[3];
  p[0] = 1.0;
  p[1] = 2.0;
  p[2] = 3.0;

  double normalProjection[3];
  transform->TransformPoint(p, normalProjection);

  std::cout << "Standard projection: " << normalProjection[0] << " "
            << normalProjection[1] << " " << normalProjection[2] << std::endl;

  double perspectiveProjection[3];
  perspectiveTransform->TransformPoint(p, perspectiveProjection);
  std::cout << "Perspective projection: " << perspectiveProjection[0] << " "
            << perspectiveProjection[1] << " " << perspectiveProjection[2]
            << std::endl;

  return EXIT_SUCCESS;
}
