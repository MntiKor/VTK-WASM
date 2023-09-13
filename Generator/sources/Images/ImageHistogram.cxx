#include <vtkCamera.h>
#include <vtkImageData.h>
#include <vtkImageHistogram.h>
#include <vtkImageProperty.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkInteractorStyleImage.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>

int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " filename e.g. Pileated.jpg"
              << std::endl;
    return EXIT_FAILURE;
  }
  vtkNew<vtkRenderWindowInteractor> iren;
  vtkNew<vtkInteractorStyle> style;
  vtkNew<vtkRenderWindow> renWin;
  iren->SetRenderWindow(renWin);
  iren->SetInteractorStyle(style);

  vtkNew<vtkImageReader2Factory> readerFactory;
  vtkSmartPointer<vtkImageReader2> reader;
  reader.TakeReference(readerFactory->CreateImageReader2(argv[1]));
  reader->SetFileName(argv[1]);

  vtkNew<vtkImageHistogram> histogram;
  histogram->SetInputConnection(reader->GetOutputPort());
  histogram->GenerateHistogramImageOn();
  histogram->SetHistogramImageSize(256, 256);
  histogram->SetHistogramImageScaleToSqrt();
  histogram->AutomaticBinningOn();
  histogram->Update();

  vtkIdType nbins = histogram->GetNumberOfBins();
  double range[2];
  range[0] = histogram->GetBinOrigin();
  range[1] = range[0] + (nbins - 1) * histogram->GetBinSpacing();

  for (int i = 0; i < 2; i++)
  {
    vtkNew<vtkRenderer> renderer;
    vtkCamera* camera = renderer->GetActiveCamera();
    renderer->SetViewport(0.5 * (i & 1), 0.0, 0.5 + 0.5 * (i & 1), 1.0);
    renWin->AddRenderer(renderer);

    vtkNew<vtkImageSliceMapper> imageMapper;
    // compute y range
    double yd;
    if ((i & 1) == 0)
    {
      imageMapper->SetInputConnection(reader->GetOutputPort());
      int extent[6];
      reader->GetOutput()->GetExtent(extent);
      yd = (extent[3] - extent[2] + 1);
    }
    else
    {
      imageMapper->SetInputConnection(histogram->GetOutputPort());
      imageMapper->BorderOn();
      int extent[6];
      histogram->GetOutput()->GetExtent(extent);
      yd = (extent[3] - extent[2] + 1);
    }

    const double* bounds = imageMapper->GetBounds();
    double point[3];
    point[0] = 0.5 * (bounds[0] + bounds[1]);
    point[1] = 0.5 * (bounds[2] + bounds[3]);
    point[2] = 0.5 * (bounds[4] + bounds[5]);

    camera->SetFocalPoint(point);
    point[imageMapper->GetOrientation()] += 1000;
    camera->SetPosition(point);
    camera->SetViewUp(0.0, 1.0, 0.0);
    camera->ParallelProjectionOn();
    // Set scale so that vertical dimension fills the window
    camera->SetParallelScale(0.5 * yd);

    vtkNew<vtkImageSlice> image;
    image->SetMapper(imageMapper);

    renderer->AddViewProp(image);

    if ((i & 1) == 0)
    {
      image->GetProperty()->SetColorWindow(range[1] - range[0]);
      image->GetProperty()->SetColorLevel(0.5 * (range[0] + range[1]));
    }
    else
    {
      image->GetProperty()->SetInterpolationTypeToNearest();
      image->GetProperty()->SetColorWindow(255.0);
      image->GetProperty()->SetColorLevel(127.5);
    }
  }

  renWin->SetSize(640, 480);
  renWin->SetWindowName("ImageHistogram");

  iren->Initialize();
  renWin->Render();
  iren->Start();

  return EXIT_SUCCESS;
}
