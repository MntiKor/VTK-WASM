#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkMath.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkParametricFunctionSource.h>
#include <vtkParametricSuperToroid.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkSliderWidget.h>

#include <array>

// These callbacks do the actual work.
// Callbacks for the interactions
class SliderCallbackN1 : public vtkCommand
{
public:
  static SliderCallbackN1* New()
  {
    return new SliderCallbackN1;
  }
  virtual void Execute(vtkObject* caller, unsigned long, void*)
  {
    vtkSliderWidget* sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
    double value = static_cast<vtkSliderRepresentation2D*>(
                       sliderWidget->GetRepresentation())
                       ->GetValue();
    this->SuperToroid->SetN1(value);
  }
  SliderCallbackN1() : SuperToroid(0)
  {
  }
  vtkParametricSuperToroid* SuperToroid;
};

class SliderCallbackN2 : public vtkCommand
{
public:
  static SliderCallbackN2* New()
  {
    return new SliderCallbackN2;
  }
  virtual void Execute(vtkObject* caller, unsigned long, void*)
  {
    vtkSliderWidget* sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
    double value = static_cast<vtkSliderRepresentation2D*>(
                       sliderWidget->GetRepresentation())
                       ->GetValue();
    this->SuperToroid->SetN2(value);
  }
  SliderCallbackN2() : SuperToroid(0)
  {
  }
  vtkParametricSuperToroid* SuperToroid;
};

int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;

  // Set the background color.
  std::array<unsigned char, 4> bkg{{26, 51, 102, 255}};
  colors->SetColor("BkgColor", bkg.data());

  vtkNew<vtkParametricSuperToroid> surface;
  vtkNew<vtkParametricFunctionSource> source;

  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkPolyDataMapper> mapper;
  vtkNew<vtkActor> actor;

  vtkNew<vtkProperty> backProperty;
  backProperty->SetColor(colors->GetColor3d("Tomato").GetData());

  // Create a parametric function source, renderer, mapper, and actor
  source->SetParametricFunction(surface);

  mapper->SetInputConnection(source->GetOutputPort());

  actor->SetMapper(mapper);
  actor->SetBackfaceProperty(backProperty);
  actor->GetProperty()->SetDiffuseColor(colors->GetColor3d("Banana").GetData());
  actor->GetProperty()->SetSpecular(.5);
  actor->GetProperty()->SetSpecularPower(20);

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->SetWindowName("ParametricSuperToroidDemo");
  renderWindow->AddRenderer(renderer);
  renderWindow->SetSize(640, 480);
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("BkgColor").GetData());
  renderer->ResetCamera();
  renderer->GetActiveCamera()->Azimuth(30);
  renderer->GetActiveCamera()->Elevation(-30);
  renderer->GetActiveCamera()->Zoom(0.9);
  renderer->ResetCameraClippingRange();

  vtkNew<vtkRenderWindowInteractor> interactor;
  interactor->SetRenderWindow(renderWindow);

  // Setup a slider widget for each varying parameter
  double tubeWidth(.008);
  double sliderLength(.008);
  double titleHeight(.04);
  double labelHeight(.04);

  vtkNew<vtkSliderRepresentation2D> sliderRepN1;

  sliderRepN1->SetMinimumValue(0.0);
  sliderRepN1->SetMaximumValue(4.0);
  sliderRepN1->SetValue(1.0);
  sliderRepN1->SetTitleText("Z squareness");

  sliderRepN1->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
  sliderRepN1->GetPoint1Coordinate()->SetValue(.1, .1);
  sliderRepN1->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
  sliderRepN1->GetPoint2Coordinate()->SetValue(.9, .1);

  sliderRepN1->SetTubeWidth(tubeWidth);
  sliderRepN1->SetSliderLength(sliderLength);
  sliderRepN1->SetTitleHeight(titleHeight);
  sliderRepN1->SetLabelHeight(labelHeight);

  vtkNew<vtkSliderWidget> sliderWidgetN1;
  sliderWidgetN1->SetInteractor(interactor);
  sliderWidgetN1->SetRepresentation(sliderRepN1);
  sliderWidgetN1->SetAnimationModeToAnimate();
  sliderWidgetN1->EnabledOn();

  vtkNew<SliderCallbackN1> callbackN1;
  callbackN1->SuperToroid = surface;

  sliderWidgetN1->AddObserver(vtkCommand::InteractionEvent, callbackN1);

  vtkNew<vtkSliderRepresentation2D> sliderRepN2;

  sliderRepN2->SetMinimumValue(0.0001);
  sliderRepN2->SetMaximumValue(4.0);
  sliderRepN2->SetValue(1.0);
  sliderRepN2->SetTitleText("XY squareness");

  sliderRepN2->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
  sliderRepN2->GetPoint1Coordinate()->SetValue(.1, .9);
  sliderRepN2->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
  sliderRepN2->GetPoint2Coordinate()->SetValue(.9, .9);

  sliderRepN2->SetTubeWidth(tubeWidth);
  sliderRepN2->SetSliderLength(sliderLength);
  sliderRepN2->SetTitleHeight(titleHeight);
  sliderRepN2->SetLabelHeight(labelHeight);

  vtkNew<vtkSliderWidget> sliderWidgetN2;
  sliderWidgetN2->SetInteractor(interactor);
  sliderWidgetN2->SetRepresentation(sliderRepN2);
  sliderWidgetN2->SetAnimationModeToAnimate();
  sliderWidgetN2->EnabledOn();

  vtkNew<SliderCallbackN2> callbackN2;
  callbackN2->SuperToroid = surface;

  sliderWidgetN2->AddObserver(vtkCommand::InteractionEvent, callbackN2);

  vtkNew<vtkSliderRepresentation2D> sliderRepMinimumV;

  sliderRepN1->SetMinimumValue(.0001);
  sliderRepMinimumV->SetMaximumValue(.9999 * vtkMath::Pi());
  sliderRepMinimumV->SetValue(.0001);
  sliderRepMinimumV->SetTitleText("V min");

  sliderRepMinimumV->GetPoint1Coordinate()
      ->SetCoordinateSystemToNormalizedDisplay();
  sliderRepMinimumV->GetPoint1Coordinate()->SetValue(.1, .1);
  sliderRepMinimumV->GetPoint2Coordinate()
      ->SetCoordinateSystemToNormalizedDisplay();
  sliderRepMinimumV->GetPoint2Coordinate()->SetValue(.1, .9);

  sliderRepMinimumV->SetTubeWidth(tubeWidth);
  sliderRepMinimumV->SetSliderLength(sliderLength);
  sliderRepMinimumV->SetTitleHeight(titleHeight);
  sliderRepMinimumV->SetLabelHeight(labelHeight);

  surface->SetN1(1.0);
  surface->SetN2(1.0);

  renderer->ResetCamera();
  renderer->GetActiveCamera()->Azimuth(30);
  renderer->GetActiveCamera()->Elevation(-30);
  renderer->GetActiveCamera()->Zoom(0.9);
  renderer->ResetCameraClippingRange();
  renderWindow->Render();

  interactor->Initialize();

  interactor->Start();

  return EXIT_SUCCESS;
}
