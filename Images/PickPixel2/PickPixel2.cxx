#include <vtkAssemblyPath.h>
#include <vtkCell.h>
#include <vtkCommand.h>
#include <vtkCornerAnnotation.h>
#include <vtkImageActor.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageNoiseSource.h>
#include <vtkImageViewer2.h>
#include <vtkInteractorStyleImage.h>
#include <vtkMath.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPropPicker.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTIFFReader.h>
#include <vtkTextProperty.h>

namespace {

// Template for image value reading.
template <typename T>
void vtkValueMessageTemplate(vtkImageData* image, int* position,
                             std::string& message)
{
  T* tuple = ((T*)image->GetScalarPointer(position));
  int components = image->GetNumberOfScalarComponents();
  for (int c = 0; c < components; ++c)
  {
    message += vtkVariant(tuple[c]).ToString();
    if (c != (components - 1))
    {
      message += ", ";
    }
  }
  message += " )";
}

// The mouse motion callback, to pick the image and recover pixel values.
class vtkImageInteractionCallback : public vtkCommand
{
public:
  static vtkImageInteractionCallback* New()
  {
    return new vtkImageInteractionCallback;
  }

  vtkImageInteractionCallback()
  {
    this->Viewer = NULL;
    this->Picker = NULL;
    this->Annotation = NULL;
  }

  ~vtkImageInteractionCallback()
  {
    this->Viewer = NULL;
    this->Picker = NULL;
    this->Annotation = NULL;
  }

  void SetPicker(vtkPropPicker* picker)
  {
    this->Picker = picker;
  }

  void SetAnnotation(vtkCornerAnnotation* annotation)
  {
    this->Annotation = annotation;
  }

  void SetViewer(vtkImageViewer2* viewer)
  {
    this->Viewer = viewer;
  }

  virtual void Execute(vtkObject*, unsigned long vtkNotUsed(event), void*)
  {
    vtkRenderWindowInteractor* interactor =
        this->Viewer->GetRenderWindow()->GetInteractor();
    vtkRenderer* renderer = this->Viewer->GetRenderer();
    vtkImageActor* actor = this->Viewer->GetImageActor();
    vtkImageData* image = this->Viewer->GetInput();
    vtkInteractorStyle* style =
        dynamic_cast<vtkInteractorStyle*>(interactor->GetInteractorStyle());

    // Pick at the mouse location provided by the interactor.
    this->Picker->Pick(interactor->GetEventPosition()[0],
                       interactor->GetEventPosition()[1], 0.0, renderer);

    // There could be other props assigned to this picker, so
    // make sure we picked the image actor.
    vtkAssemblyPath* path = this->Picker->GetPath();
    bool validPick = false;

    if (path)
    {
      vtkCollectionSimpleIterator sit;
      path->InitTraversal(sit);
      for (int i = 0; i < path->GetNumberOfItems() && !validPick; ++i)
      {
        auto node = path->GetNextNode(sit);
        if (actor == dynamic_cast<vtkImageActor*>(node->GetViewProp()))
        {
          validPick = true;
        }
      }
    }

    if (!validPick)
    {
      this->Annotation->SetText(0, "Off Image");
      interactor->Render();
      // Pass the event further on.
      style->OnMouseMove();
      return;
    }

    // Get the world coordinates of the pick.
    double pos[3];
    this->Picker->GetPickPosition(pos);

    int image_coordinate[3];

    int axis = this->Viewer->GetSliceOrientation();
    switch (axis)
    {
    case vtkImageViewer2::SLICE_ORIENTATION_XZ:
      image_coordinate[0] = vtkMath::Round(pos[0]);
      image_coordinate[1] = this->Viewer->GetSlice();
      image_coordinate[2] = vtkMath::Round(pos[2]);
      break;
    case vtkImageViewer2::SLICE_ORIENTATION_YZ:
      image_coordinate[0] = this->Viewer->GetSlice();
      image_coordinate[1] = vtkMath::Round(pos[0]);
      image_coordinate[2] = vtkMath::Round(pos[1]);
      break;
    default: // vtkImageViewer2::SLICE_ORIENTATION_XY
      image_coordinate[0] = vtkMath::Round(pos[0]);
      image_coordinate[1] = vtkMath::Round(pos[1]);
      image_coordinate[2] = this->Viewer->GetSlice();
      break;
    }

    std::string message = "Location: ( ";
    message += vtkVariant(image_coordinate[0]).ToString();
    message += ", ";
    message += vtkVariant(image_coordinate[1]).ToString();
    message += ", ";
    message += vtkVariant(image_coordinate[2]).ToString();
    message += " )\nValue: ( ";

    switch (image->GetScalarType())
    {
      vtkTemplateMacro(
          (vtkValueMessageTemplate<VTK_TT>(image, image_coordinate, message)));

    default:
      return;
    }

    this->Annotation->SetText(0, message.c_str());
    interactor->Render();
    style->OnMouseMove();
  }

private:
  vtkImageViewer2* Viewer;         // Pointer to the viewer
  vtkPropPicker* Picker;           // Pointer to the picker
  vtkCornerAnnotation* Annotation; // Pointer to the annotation
};

} // namespace

int main(int argc, char* argv[])
{
  vtkNew<vtkNamedColors> colors;

  vtkNew<vtkImageViewer2> imageViewer;

  // Verify input arguments.
  if (argc != 2)
  {
    std::cout << argv[0]
              << " Required parameters: (tif) Filename e.g. ColorCells.tif"
              << std::endl
              << "missing..." << std::endl;
    std::cout << "A noise image will be created!" << std::endl;

    // Create a noise image.
    vtkNew<vtkImageNoiseSource> noiseSource;
    noiseSource->SetWholeExtent(0, 512, 0, 512, 0, 0);
    noiseSource->SetMinimum(0.0);
    noiseSource->SetMaximum(255.0);

    // Cast noise image to unsigned char.
    vtkNew<vtkImageCast> imageCast;
    imageCast->SetInputConnection(noiseSource->GetOutputPort());
    imageCast->SetOutputScalarTypeToUnsignedChar();
    imageCast->Update();

    // Connect to image viewer pipeline.
    imageViewer->SetInputConnection(imageCast->GetOutputPort());
  }
  else
  {
    // Parse input argument.
    std::string inputFilename = argv[1];

    // Read the image.
    vtkNew<vtkTIFFReader> tiffReader;
    if (!tiffReader->CanReadFile(inputFilename.c_str()))
    {
      std::cout << argv[0] << ": Error reading file " << inputFilename
                << std::endl;
      return EXIT_FAILURE;
    }
    tiffReader->SetFileName(inputFilename.c_str());

    // Connect to image viewer pipeline.
    imageViewer->SetInputConnection(tiffReader->GetOutputPort());
  }

  // Picker to pick pixels.
  vtkNew<vtkPropPicker> propPicker;
  propPicker->PickFromListOn();

  // Give the picker a prop to pick.
  vtkImageActor* imageActor = imageViewer->GetImageActor();
  propPicker->AddPickList(imageActor);

  // Disable interpolation, so we can see each pixel.
  imageActor->InterpolateOff();

  // Visualize.
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  imageViewer->SetupInteractor(renderWindowInteractor);
  imageViewer->SetSize(600, 600);
  imageViewer->GetRenderWindow()->SetWindowName("PickPixel2");

  vtkRenderer* renderer = imageViewer->GetRenderer();
  renderer->ResetCamera();
  renderer->GradientBackgroundOn();
  renderer->SetBackground(colors->GetColor3d("DarkSlateGray").GetData());
  renderer->SetBackground2(colors->GetColor3d("LightSlateGray").GetData());

  // Annotate the image with window/level and mouse over pixel
  // information
  vtkNew<vtkCornerAnnotation> cornerAnnotation;
  cornerAnnotation->SetLinearFontScaleFactor(2);
  cornerAnnotation->SetNonlinearFontScaleFactor(1);
  cornerAnnotation->SetMaximumFontSize(20);
  cornerAnnotation->SetText(0, "Off Image");
  cornerAnnotation->SetText(3, "<window>\n<level>");
  cornerAnnotation->GetTextProperty()->SetColor(
      colors->GetColor3d("LightGoldenrodYellow").GetData());

  imageViewer->GetRenderer()->AddViewProp(cornerAnnotation);

  // Callback listens to MouseMoveEvents invoked by the interactor's style
  vtkNew<vtkImageInteractionCallback> callback;
  callback->SetViewer(imageViewer);
  callback->SetAnnotation(cornerAnnotation);
  callback->SetPicker(propPicker);

  // InteractorStyleImage allows for the following controls:
  // 1) middle mouse + move = camera pan
  // 2) left mouse + move = window/level
  // 3) right mouse + move = camera zoom
  // 4) middle mouse wheel scroll = zoom
  // 5) 'r' = reset window/level
  // 6) shift + 'r' = reset camera
  vtkInteractorStyleImage* imageStyle = imageViewer->GetInteractorStyle();
  imageStyle->AddObserver(vtkCommand::MouseMoveEvent, callback);

  renderWindowInteractor->Initialize();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
