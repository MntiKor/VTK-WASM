#include <vtkAssemblyNode.h>
#include <vtkAssemblyPath.h>
#include <vtkBorderRepresentation.h>
#include <vtkBorderWidget.h>
#include <vtkCommand.h>
#include <vtkCoordinate.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkInteractorStyleImage.h>
#include <vtkJPEGReader.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPropPicker.h>
#include <vtkProperty2D.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

class vtkBorderCallback : public vtkCommand
{
public:
  vtkBorderCallback()
  {
  }

  static vtkBorderCallback* New()
  {
    return new vtkBorderCallback;
  }

  virtual void Execute(vtkObject* caller, unsigned long, void*)
  {
    vtkBorderWidget* borderWidget = reinterpret_cast<vtkBorderWidget*>(caller);

    // Get the display coordinates of the two corners of the box
    vtkCoordinate* lowerLeftCoordinate =
        static_cast<vtkBorderRepresentation*>(borderWidget->GetRepresentation())
            ->GetPositionCoordinate();
    double* lowerLeft;
    lowerLeft = lowerLeftCoordinate->GetComputedWorldValue(this->Renderer);
    std::cout << "Lower left coordinate: " << lowerLeft[0] << ", "
              << lowerLeft[1] << std::endl;
    lowerLeft[2] = 0;

    vtkCoordinate* upperRightCoordinate =
        static_cast<vtkBorderRepresentation*>(borderWidget->GetRepresentation())
            ->GetPosition2Coordinate();
    double* upperRight;
    upperRight = upperRightCoordinate->GetComputedWorldValue(this->Renderer);
    std::cout << "Upper right coordinate: " << upperRight[0] << ", "
              << upperRight[1] << std::endl;
    upperRight[2] = 0;

    // Pick at the two corners of the box.
    vtkSmartPointer<vtkPropPicker> picker =
        vtkSmartPointer<vtkPropPicker>::New();

    //    int valid1 = picker->Pick( lowerLeft[0],
    picker->Pick(lowerLeft[0], lowerLeft[1], 0.0, this->Renderer);

    vtkAssemblyPath* path = picker->GetPath();

    vtkProp* pickedProp = NULL;

    bool validPick = false;
    if (path)
    {
      std::cout << "There are " << path->GetNumberOfItems()
                << " items in the path." << std::endl;
      vtkCollectionSimpleIterator sit;
      path->InitTraversal(sit);
      vtkAssemblyNode* node;
      for (int i = 0; i < path->GetNumberOfItems(); ++i)
      {
        node = path->GetNextNode(sit);
        pickedProp = node->GetViewProp();
        if (this->ImageActor == dynamic_cast<vtkImageActor*>(pickedProp))
        {
          std::cout << "Correct actor picked." << std::endl;
          validPick = true;
          break;
        }
      }
    }

    if (!validPick)
    {
      std::cout << "Off Image" << std::endl;
    }
    else
    {
      double pos1[3];
      picker->GetPickPosition(pos1);
      std::cout << "Lower Left Pick: " << pos1[0] << " " << pos1[1]
                << std::endl;
    }
    /*
      if(valid1 != 0)
        {
        std::cout << "Valid1: " << valid1 << std::endl;
        double pos1[3];
        picker->GetPickPosition( pos1 );
        std::cout << "Lower Left Pick: " << pos1[0] << " " << pos1[1] <<
      std::endl;
        }
      else
        {
        std::cout << "Left - nothing was picked" << std::endl;
        }
      */

    /*
    int valid2 = picker->Pick( upperRight[0],
                  upperRight[1], 0.0, this->Renderer );
    if(valid2 != 0)
      {
      std::cout << "Valid2: " << valid2 << std::endl;
      double pos2[3];
      picker->GetPickPosition( pos2 );
      std::cout << "Upper right Pick: " << pos2[0] << " " << pos2[1] <<
    std::endl;
      }
    else
      {
      std::cout << "Right - nothing was picked" << std::endl;
      }
    */
    std::cout << std::endl;
  }

  void SetRenderer(vtkSmartPointer<vtkRenderer> ren)
  {
    this->Renderer = ren;
  }
  void SetImageActor(vtkSmartPointer<vtkImageActor> im)
  {
    this->ImageActor = im;
  }

private:
  vtkSmartPointer<vtkRenderer> Renderer;
  vtkSmartPointer<vtkImageActor> ImageActor;
};

int main(int argc, char* argv[])
{
  vtkNew<vtkNamedColors> color;

  // Parse input arguments.
  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " Filename(.jpg) e.g. Gourds2.jpg"
              << std::endl;
    return EXIT_FAILURE;
  }

  std::string InputFilename = argv[1];

  // Read the image
  vtkNew<vtkJPEGReader> jPEGReader;
  jPEGReader->SetFileName(InputFilename.c_str());

  vtkNew<vtkImageActor> actor;
  actor->GetMapper()->SetInputConnection(jPEGReader->GetOutputPort());

  // A renderer and render window
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);

  // An interactor
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  vtkNew<vtkInteractorStyleImage> style;

  renderWindowInteractor->SetInteractorStyle(style);

  vtkNew<vtkBorderWidget> borderWidget;
  borderWidget->SetInteractor(renderWindowInteractor);
  static_cast<vtkBorderRepresentation*>(borderWidget->GetRepresentation())
      ->GetBorderProperty()
      ->SetColor(color->GetColor3d("Lime").GetData());
  borderWidget->SelectableOff();

  vtkNew<vtkBorderCallback> borderCallback;
  borderCallback->SetRenderer(renderer);
  borderCallback->SetImageActor(actor);

  borderWidget->AddObserver(vtkCommand::InteractionEvent, borderCallback);

  // Add the actors to the scene
  renderer->AddActor(actor);
  renderer->SetBackground(color->GetColor3d("DarkSlateGray").GetData());

  renderWindow->SetWindowName("ImageRegion");
  renderWindow->Render();
  renderWindowInteractor->Initialize();
  renderWindow->Render();
  borderWidget->On();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
