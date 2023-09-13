#include <vtkDataObject.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

class TestClass : public vtkDataObject
{
public:
  static TestClass* New();
  vtkTypeRevisionMacro(TestClass, vtkDataObject);
  TestClass()
  {
    vtkWarningMacro("Test warning.");
  }
};

vtkStandardNewMacro(TestClass);

int main(int, char*[])
{

  vtkSmartPointer<TestClass> test = vtkSmartPointer<TestClass>::New();

  return EXIT_SUCCESS;
}
