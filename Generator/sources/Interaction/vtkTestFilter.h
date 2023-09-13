#ifndef __vtkTestFilter_h
#define __vtkTestFilter_h

#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkPolyDataAlgorithm.h>

class vtkTestFilter : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkTestFilter, vtkPolyDataAlgorithm);
  static vtkTestFilter* New();

  int RefreshEvent;

protected:
  vtkTestFilter();

  int RequestData(vtkInformation*, vtkInformationVector**,
                  vtkInformationVector*) override;

private:
  vtkTestFilter(const vtkTestFilter&);  // Not implemented.
  void operator=(const vtkTestFilter&); // Not implemented.
};

vtkStandardNewMacro(vtkTestFilter);

vtkTestFilter::vtkTestFilter()
{
  this->SetNumberOfInputPorts(0);

  this->RefreshEvent = vtkCommand::UserEvent + 1;
}

int vtkTestFilter::RequestData(vtkInformation* vtkNotUsed(request),
                               vtkInformationVector**, vtkInformationVector*)
{
  // Get the info object
  //  auto outInfo = outputVector->GetInformationObject(0);

  //  auto output = vtkPolyData::SafeDownCast(
  //      outInfo->Get(vtkDataObject::DATA_OBJECT()));
  this->InvokeEvent(this->RefreshEvent, nullptr);

  return 1;
}

#endif // __vtkTestFilter_h
