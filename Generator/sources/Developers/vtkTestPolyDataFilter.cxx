#include <vtkTestPolyDataFilter.h>

#include <vtkDataObject.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkStreamingDemandDrivenPipeline.h>

vtkStandardNewMacro(vtkTestPolyDataFilter);

vtkTestPolyDataFilter::vtkTestPolyDataFilter()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

vtkTestPolyDataFilter::~vtkTestPolyDataFilter()
{
}

int vtkTestPolyDataFilter::RequestData(vtkInformation* vtkNotUsed(request),
                                       vtkInformationVector** inputVector,
                                       vtkInformationVector* outputVector)
{

  // get the input and output
  vtkPolyData* input = vtkPolyData::GetData(inputVector[0], 0);
  vtkPolyData* output = vtkPolyData::GetData(outputVector, 0);

  input->GetPoints()->InsertNextPoint(1.0, 1.0, 1.0);

  output->ShallowCopy(input);

  return 1;
}

//----------------------------------------------------------------------------
void vtkTestPolyDataFilter::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
