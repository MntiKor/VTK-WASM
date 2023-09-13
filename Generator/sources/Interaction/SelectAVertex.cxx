#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkInteractorStyleTrackballActor.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPointPicker.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVertexGlyphFilter.h>

namespace {

// Define interaction style.
class InteractorStyle2 : public vtkInteractorStyleTrackballActor
{
public:
  static InteractorStyle2* New();
  vtkTypeMacro(InteractorStyle2, vtkInteractorStyleTrackballActor);
  vtkNew<vtkNamedColors> color;

  InteractorStyle2()
  {
    this->Move = false;
    this->PointPicker = vtkSmartPointer<vtkPointPicker>::New();

    // Setup ghost glyph.
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    points->InsertNextPoint(0, 0, 0);
    this->MovePolyData = vtkSmartPointer<vtkPolyData>::New();
    this->MovePolyData->SetPoints(points);
    this->MoveGlyphFilter = vtkSmartPointer<vtkVertexGlyphFilter>::New();
    this->MoveGlyphFilter->SetInputData(this->MovePolyData);
    this->MoveGlyphFilter->Update();

    this->MoveMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    this->MoveMapper->SetInputConnection(
        this->MoveGlyphFilter->GetOutputPort());

    this->MoveActor = vtkSmartPointer<vtkActor>::New();
    this->MoveActor->SetMapper(this->MoveMapper);
    this->MoveActor->VisibilityOff();
    this->MoveActor->GetProperty()->SetPointSize(10);
    this->MoveActor->GetProperty()->SetColor(
        this->color->GetColor3d("Pink").GetData());
  }

  void OnMouseMove() override
  {
    if (!this->Move)
    {
      return;
    }

    vtkInteractorStyleTrackballActor::OnMouseMove();
  }

  void OnMiddleButtonUp() override
  {
    this->EndPan();

    this->Move = false;
    this->MoveActor->VisibilityOff();

    this->Data->GetPoints()->SetPoint(this->SelectedPoint,
                                      this->MoveActor->GetPosition());
    this->Data->Modified();
    this->GetCurrentRenderer()->Render();
    this->GetCurrentRenderer()->GetRenderWindow()->Render();
  }
  void OnMiddleButtonDown() override
  {
    // Get the selected point.
    int x = this->Interactor->GetEventPosition()[0];
    int y = this->Interactor->GetEventPosition()[1];
    this->FindPokedRenderer(x, y);

    this->PointPicker->Pick(this->Interactor->GetEventPosition()[0],
                            this->Interactor->GetEventPosition()[1],
                            0, // always zero.
                            this->Interactor->GetRenderWindow()
                                ->GetRenderers()
                                ->GetFirstRenderer());

    if (this->PointPicker->GetPointId() >= 0)
    {
      this->StartPan();
      this->MoveActor->VisibilityOn();
      this->Move = true;
      this->SelectedPoint = this->PointPicker->GetPointId();

      std::cout << "Dragging point " << this->SelectedPoint << std::endl;

      double p[3];
      this->Data->GetPoint(this->SelectedPoint, p);
      std::cout << "p: " << p[0] << " " << p[1] << " " << p[2] << std::endl;
      this->MoveActor->SetPosition(p);

      this->GetCurrentRenderer()->AddActor(this->MoveActor);
      this->InteractionProp = this->MoveActor;
    }
  }

  vtkPolyData* Data;
  vtkPolyData* GlyphData;

  vtkSmartPointer<vtkPolyDataMapper> MoveMapper;
  vtkSmartPointer<vtkActor> MoveActor;
  vtkSmartPointer<vtkPolyData> MovePolyData;
  vtkSmartPointer<vtkVertexGlyphFilter> MoveGlyphFilter;

  vtkSmartPointer<vtkPointPicker> PointPicker;

  bool Move;
  vtkIdType SelectedPoint;
};
vtkStandardNewMacro(InteractorStyle2);

} // namespace

int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;

  vtkNew<vtkPoints> points;
  points->InsertNextPoint(0, 0, 0);
  points->InsertNextPoint(1, 0, 0);
  points->InsertNextPoint(2, 0, 0);

  vtkNew<vtkPolyData> input;
  input->SetPoints(points);

  vtkNew<vtkVertexGlyphFilter> glyphFilter;
  glyphFilter->SetInputData(input);
  glyphFilter->Update();

  // Create a mapper and actor.
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(glyphFilter->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetPointSize(10);
  actor->GetProperty()->SetColor(colors->GetColor3d("Tomato").GetData());

  // Visualize
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("SelectAVertex");

  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("Gray").GetData());

  renderWindow->Render();

  vtkNew<InteractorStyle2> style;
  renderWindowInteractor->SetInteractorStyle(style);
  style->Data = input;

  renderer->GetActiveCamera()->Zoom(0.9);

  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
