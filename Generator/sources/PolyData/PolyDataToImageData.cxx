#include <vtkImageData.h>
#include <vtkImageStencil.h>
#include <vtkMetaImageWriter.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkSphereSource.h>

/**
 * This program generates a sphere (closed surface, vtkPolyData) and converts it
 * into volume representation (vtkImageData) where the foreground voxels are 1
 * and the background voxels are 0. Internally vtkPolyDataToImageStencil is
 * utilized. The resultant image is saved to disk in metaimage file format
 * (SphereVolume.mhd).
 */
int main(int, char*[])
{
  vtkNew<vtkSphereSource> sphereSource;
  sphereSource->SetRadius(20);
  sphereSource->SetPhiResolution(30);
  sphereSource->SetThetaResolution(30);
  auto pd = sphereSource->GetOutput();
  sphereSource->Update();

  vtkNew<vtkImageData> whiteImage;
  double bounds[6];
  pd->GetBounds(bounds);
  double spacing[3]; // desired volume spacing
  spacing[0] = 0.5;
  spacing[1] = 0.5;
  spacing[2] = 0.5;
  whiteImage->SetSpacing(spacing);

  // compute dimensions
  int dim[3];
  for (int i = 0; i < 3; i++)
  {
    dim[i] = static_cast<int>(
        ceil((bounds[i * 2 + 1] - bounds[i * 2]) / spacing[i]));
  }
  whiteImage->SetDimensions(dim);
  whiteImage->SetExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);

  double origin[3];
  origin[0] = bounds[0] + spacing[0] / 2;
  origin[1] = bounds[2] + spacing[1] / 2;
  origin[2] = bounds[4] + spacing[2] / 2;
  whiteImage->SetOrigin(origin);
  whiteImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

  // Fill the image with foreground voxels:
  unsigned char inval = 255;
  unsigned char outval = 0;
  vtkIdType count = whiteImage->GetNumberOfPoints();
  for (vtkIdType i = 0; i < count; ++i)
  {
    whiteImage->GetPointData()->GetScalars()->SetTuple1(i, inval);
  }

  // polygonal data --> image stencil:
  vtkNew<vtkPolyDataToImageStencil> pol2stenc;
  pol2stenc->SetInputData(pd);
  pol2stenc->SetOutputOrigin(origin);
  pol2stenc->SetOutputSpacing(spacing);
  pol2stenc->SetOutputWholeExtent(whiteImage->GetExtent());
  pol2stenc->Update();

  // Cut the corresponding white image and set the background:
  vtkNew<vtkImageStencil> imgstenc;
  imgstenc->SetInputData(whiteImage);
  imgstenc->SetStencilConnection(pol2stenc->GetOutputPort());
  imgstenc->ReverseStencilOff();
  imgstenc->SetBackgroundValue(outval);
  imgstenc->Update();

  vtkNew<vtkMetaImageWriter> writer;
  writer->SetFileName("SphereVolume.mhd");
  writer->SetInputData(imgstenc->GetOutput());
  writer->Write();

  return EXIT_SUCCESS;
}
