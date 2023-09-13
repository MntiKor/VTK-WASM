#include <string>
#include <vector>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkXMLPolyDataReader.h>

namespace {
void FindAllData(const std::string& inputFilename);
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    std::cout << "Usage: " << argv[0]
              << " InputFilename e.g. filledContours.vtp" << std::endl;
    return EXIT_FAILURE;
  }

  std::string inputFilename = argv[1];
  FindAllData(inputFilename);

  return EXIT_SUCCESS;
}

namespace {
void FindAllData(const std::string& InputFilename)
{
  vtkNew<vtkXMLPolyDataReader> reader;
  reader->SetFileName(InputFilename.c_str());
  reader->Update();
  vtkPolyData* polydata = reader->GetOutput();

  unsigned int numberOfArrays = polydata->GetPointData()->GetNumberOfArrays();
  std::cout << "NumArrays: " << numberOfArrays << std::endl;

  std::cout << "key: " << std::endl;
  ;
  // More values can be found in <VTK_DIR>/Common/Core/vtkSetGet.h
  std::cout << VTK_UNSIGNED_CHAR << " unsigned char" << std::endl;
  std::cout << VTK_UNSIGNED_INT << " unsigned int" << std::endl;
  std::cout << VTK_FLOAT << " float" << std::endl;
  std::cout << VTK_DOUBLE << " double" << std::endl;

  std::vector<std::string> arrayNames;
  for (unsigned int i = 0; i < numberOfArrays; i++)
  {
    // The following two lines are equivalent:
    // arrayNames.push_back(polydata->GetPointData()->GetArray(i)->GetName());
    arrayNames.push_back(polydata->GetPointData()->GetArrayName(i));
    int dataTypeID = polydata->GetPointData()->GetArray(i)->GetDataType();
    std::cout << "Array " << i << ": " << arrayNames[i]
              << " (type: " << dataTypeID << ")" << std::endl;
  }
}
} // namespace
