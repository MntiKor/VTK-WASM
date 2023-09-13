#include <vtkActor.h>
#include <vtkCallbackCommand.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkSliderWidget.h>
#include <vtkSphereSource.h>
#include <vtkTextProperty.h>

namespace {
// This does the actual work.
// Callback for the interaction.
class vtkSliderCallback : public vtkCallbackCommand
{
public:
  static vtkSliderCallback* New()
  {
    return new vtkSliderCallback;
  }
  virtual void Execute(vtkObject* caller, unsigned long, void*)
  {
    vtkSliderWidget* sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
    this->SphereSource->SetPhiResolution(
        static_cast<vtkSliderRepresentation*>(sliderWidget->GetRepresentation())
            ->GetValue());
    this->SphereSource->SetThetaResolution(
        static_cast<vtkSliderRepresentation*>(sliderWidget->GetRepresentation())
            ->GetValue());
  }
  vtkSliderCallback() : SphereSource(0)
  {
  }
  vtkSphereSource* SphereSource;
};
} // namespace

int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;

  // A sphere.
  vtkNew<vtkSphereSource> sphereSource;
  sphereSource->SetCenter(0.0, 0.0, 0.0);
  sphereSource->SetRadius(4.0);

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(sphereSource->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetInterpolationToFlat();
  actor->GetProperty()->SetColor(colors->GetColor3d("MistyRose").GetData());
  actor->GetProperty()->SetEdgeColor(colors->GetColor3d("Tomato").GetData());
  actor->GetProperty()->EdgeVisibilityOn();

  // A renderer and render window.
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("Slider2D");
  renderWindow->SetSize(640, 480);

  // An interactor
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  // Add the actors to the scene.
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("SlateGray").GetData());

  // Render an image (lights and cameras are created automatically).
  renderWindow->Render();

  vtkNew<vtkSliderRepresentation2D> sliderRep;

  sliderRep->SetMinimumValue(3.0);
  sliderRep->SetMaximumValue(20.0);
  sliderRep->SetValue(5.0);
  sliderRep->SetTitleText("Sphere Resolution");

  // Set color properties:
  //   // Set color properties:
  // Change the color of the knob that slides
  sliderRep->GetSliderProperty()->SetColor(
      colors->GetColor3d("Green").GetData());
  // Change the color of the text indicating what the slider controls
  sliderRep->GetTitleProperty()->SetColor(
      colors->GetColor3d("AliceBlue").GetData());
  // Change the color of the text displaying the value
  sliderRep->GetLabelProperty()->SetColor(
      colors->GetColor3d("AliceBlue").GetData());
  // Change the color of the knob when the mouse is held on it
  sliderRep->GetSelectedProperty()->SetColor(
      colors->GetColor3d("DeepPink").GetData());
  // Change the color of the bar
  sliderRep->GetTubeProperty()->SetColor(
      colors->GetColor3d("MistyRose").GetData());
  // Change the color of the ends of the bar
  sliderRep->GetCapProperty()->SetColor(colors->GetColor3d("Yellow").GetData());
  sliderRep->SetSliderLength(0.05);
  sliderRep->SetSliderWidth(0.025);
  sliderRep->SetEndCapLength(0.02);

  // Display pixel values (640 X 480)
  // sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
  // sliderRep->GetPoint1Coordinate()->SetValue(128, 48);
  // sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
  // sliderRep->GetPoint2Coordinate()->SetValue(512, 48);
  // Or use this - better because it scales to the window size:
  sliderRep->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
  sliderRep->GetPoint1Coordinate()->SetValue(0.2, 0.1);
  sliderRep->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
  sliderRep->GetPoint2Coordinate()->SetValue(0.8, 0.1);

  vtkNew<vtkSliderWidget> sliderWidget;
  sliderWidget->SetInteractor(renderWindowInteractor);
  sliderWidget->SetRepresentation(sliderRep);
  sliderWidget->SetAnimationModeToAnimate();
  sliderWidget->EnabledOn();

  vtkNew<vtkSliderCallback> callback;
  callback->SphereSource = sphereSource;

  sliderWidget->AddObserver(vtkCommand::InteractionEvent, callback);

  renderWindowInteractor->Initialize();
  renderWindow->Render();

  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
