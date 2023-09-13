#include <vtkImageCanvasSource2D.h>
#include <vtkMPEG2Writer.h>
#include <vtkNew.h>

int main(int argc, char* argv[])
{
  vtkNew<vtkImageCanvasSource2D> source;
  source->SetScalarTypeToUnsignedChar();
  source->SetNumberOfScalarComponents(3);
  source->SetExtent(0, 100, 0, 100, 0, 0);

  vtkNew<vtkMPEG2Writer> writer;

  writer->SetInputConnection(source->GetOutputPort());
  writer->SetFileName("test.avi");
  writer->Start();

  for (unsigned int i = 0; i < 100; i++)
  {
    source->SetDrawColor(0, 0, 0, 1);   // black
    source->FillBox(0, 100, 0, 100);    // clear image
    source->SetDrawColor(255, 0, 0, 1); // red
    source->FillBox(i, 20, 10, 20);
    source->Update();

    writer->Write();
  }
  writer->End();

  return EXIT_SUCCESS;
}
