//
// This example demonstrates how to access the source object
// (e.g. vtkSphereSource) from the actor reversely.
//
// Some standard vtk headers.
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkMath.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>

// Additionally needed vtk header for this example.
#include <vtkAlgorithmOutput.h>

int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;

  // Source
  vtkNew<vtkSphereSource> sphere;
  sphere->SetRadius(0.5);
  // Mapper
  vtkNew<vtkPolyDataMapper> sphereMapper;
  sphereMapper->SetInputConnection(sphere->GetOutputPort());
  // Actor
  vtkNew<vtkActor> sphereActor;
  sphereActor->SetMapper(sphereMapper);
  sphereActor->GetProperty()->SetColor(
      colors->GetColor3d("MistyRose").GetData());

  // Renderer
  vtkNew<vtkRenderer> ren1;
  ren1->SetBackground(colors->GetColor3d("CadetBlue").GetData());

  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(ren1);
  renWin->SetSize(300, 300);
  renWin->SetWindowName("ReverseAccess");

  // Add actor to the renderer.
  ren1->AddActor(sphereActor);

  //
  // Now we retrieve the source object from vtkActor reversely,
  // meaning we don't use the spheresource object we instantiated
  // above directly,
  // instead we retrieve a reference to the spheresource through the
  // actor.
  // An advantage of this concept might be that we don't need to
  // maintain the source object anymore
  // in a more complex application.
  // To demonstrate that we can modify properties of the spheresource
  // through this reference
  // beside changing some properties of the actor (in this example we
  // change actor's x-position),
  // we change the radius of the spheresource as well.
  //
  // The next two lines are the core lines for reverse access.
  //
  vtkSmartPointer<vtkAlgorithm> algorithm =
      sphereActor->GetMapper()->GetInputConnection(0, 0)->GetProducer();
  auto srcReference = dynamic_cast<vtkSphereSource*>(algorithm.GetPointer());

  float origRadius = srcReference->GetRadius();
  for (int i = 0; i < 360; ++i)
  {
    // Change radius of the sphere source.
    srcReference->SetRadius(origRadius *
                            (1 + sin((float)i / 180.0 * vtkMath::Pi())));
    // Change the x-position of the actor.
    sphereActor->SetPosition(sin((float)i / 45.0 * vtkMath::Pi()) * 0.5, 0, 0);
    renWin->Render();
  }

  //
  // Thanks to the usage of vtkSmartPointer there is no explicit need
  // to free any objects at this point.
  //
  return EXIT_SUCCESS;
}
