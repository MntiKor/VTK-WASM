#include <vtkImageActor.h>
#include <vtkImageCast.h>
#include <vtkImageMandelbrotSource.h>
#include <vtkImageMapper3D.h>
#include <vtkMetaImageReader.h>
#include <vtkMetaImageWriter.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

int main(int, char*[])
{
  vtkNew<vtkNamedColors> colors;

  // adapt path !
  std::string filePath = "julia_mha.mhd";
  std::string filePathRaw = "julia_mha.raw";
  // Create an image
  vtkNew<vtkImageMandelbrotSource> source;

  vtkNew<vtkImageCast> castFilter;
  castFilter->SetOutputScalarTypeToUnsignedChar();
  castFilter->SetInputConnection(source->GetOutputPort());
  castFilter->Update();

  vtkNew<vtkMetaImageWriter> writer;
  writer->SetInputConnection(castFilter->GetOutputPort());
  writer->SetFileName(filePath.c_str());
  writer->SetRAWFileName(filePathRaw.c_str());
  writer->Write();

  // Read and display file for verification that it was written correctly
  vtkNew<vtkMetaImageReader> reader;
  reader->SetFileName(filePath.c_str());
  reader->Update();

  vtkNew<vtkImageActor> actor;
  actor->GetMapper()->SetInputConnection(reader->GetOutputPort());

  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("SteelBlue").GetData());

  renderWindow->SetWindowName("MetaImageWriter");
  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
