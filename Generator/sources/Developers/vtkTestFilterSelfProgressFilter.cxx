#include <vtkCallbackCommand.h>
#include <vtkDataObject.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>

#include "vtkTestFilterSelfProgressFilter.h"

vtkStandardNewMacro(vtkTestFilterSelfProgressFilter);

vtkTestFilterSelfProgressFilter::vtkTestFilterSelfProgressFilter()
{
  vtkNew<vtkCallbackCommand> progressCallback;
  progressCallback->SetCallback(this->ProgressFunction);

  this->AddObserver(vtkCommand::ProgressEvent, progressCallback);
}

void vtkTestFilterSelfProgressFilter::ProgressFunction(
    vtkObject* caller, long unsigned int /* eventId */, void* /* clientData */,
    void* /* callData */)
{
  vtkTestFilterSelfProgressFilter* testFilter =
      static_cast<vtkTestFilterSelfProgressFilter*>(caller);
  cout << "Progress: " << testFilter->GetProgress() << endl;
}

int vtkTestFilterSelfProgressFilter::RequestData(
    vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector,
    vtkInformationVector* outputVector)
{

  // get the info objects
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  // get the input and ouptut
  vtkPolyData* input =
      dynamic_cast<vtkPolyData*>(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  vtkPolyData* output =
      dynamic_cast<vtkPolyData*>(outInfo->Get(vtkDataObject::DATA_OBJECT()));

  for (vtkIdType i = 0; i < input->GetNumberOfPoints(); i++)
  {
    this->UpdateProgress(static_cast<double>(i) / input->GetNumberOfPoints());
  }

  output->ShallowCopy(input);

  return 1;
}
