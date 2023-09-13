#include "vtkTestGraphAlgorithmSource.h"

#include <vtkDataObject.h>
#include <vtkGraph.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkMutableUndirectedGraph.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkUndirectedGraph.h>

vtkStandardNewMacro(vtkTestGraphAlgorithmSource);

vtkTestGraphAlgorithmSource::vtkTestGraphAlgorithmSource()
{
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

vtkTestGraphAlgorithmSource::~vtkTestGraphAlgorithmSource()
{
}

int vtkTestGraphAlgorithmSource::RequestData(
    vtkInformation* vtkNotUsed(request),
    vtkInformationVector** vtkNotUsed(inputVector),
    vtkInformationVector* outputVector)
{

  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  vtkGraph* output =
      dynamic_cast<vtkGraph*>(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkNew<vtkMutableUndirectedGraph> NewGraph;

  // add 3 vertices
  NewGraph->AddVertex();
  NewGraph->AddVertex();
  NewGraph->AddVertex();

  output->ShallowCopy(NewGraph);

  return 1;
}

int vtkTestGraphAlgorithmSource::RequestDataObject(vtkInformation*,
                                                   vtkInformationVector**,
                                                   vtkInformationVector*)
{

  vtkNew<vtkUndirectedGraph> output;
  this->GetExecutive()->SetOutputData(0, output);

  return 1;
}

//----------------------------------------------------------------------------
void vtkTestGraphAlgorithmSource::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
