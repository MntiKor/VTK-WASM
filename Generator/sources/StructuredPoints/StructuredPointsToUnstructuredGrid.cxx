#include <vtkNew.h>
#include <vtkStructuredPoints.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridWriter.h>

int main(int, char*[])
{
  // Create a structuredpoints
  vtkNew<vtkStructuredPoints> structuredPoints;

  // Specify the size of the image data
  structuredPoints->SetDimensions(2, 3, 1);
  structuredPoints->AllocateScalars(VTK_DOUBLE, 1);

  int* dims = structuredPoints->GetDimensions();

  std::cout << "Dims: "
            << " x: " << dims[0] << " y: " << dims[1] << " z: " << dims[2]
            << std::endl;

  std::cout << "Number of points: " << structuredPoints->GetNumberOfPoints()
            << std::endl;
  std::cout << "Number of cells: " << structuredPoints->GetNumberOfCells()
            << std::endl;

  // fill every entry of the image data with "2.0"
  for (int z = 0; z < dims[2]; z++)
  {
    for (int y = 0; y < dims[1]; y++)
    {
      for (int x = 0; x < dims[0]; x++)
      {
        double* pixel =
            static_cast<double*>(structuredPoints->GetScalarPointer(x, y, z));
        pixel[0] = 2.0;
      }
    }
  }

  // Copy the points from the StructuredPoints to the UnstructuredGrid
  vtkNew<vtkPoints> points;

  // retrieve the entries from the grid and print them to the screen
  for (vtkIdType i = 0; i < structuredPoints->GetNumberOfPoints(); i++)
  {
    double p[3];
    structuredPoints->GetPoint(i, p);

    points->InsertNextPoint(p);
  }

  vtkNew<vtkUnstructuredGrid> ug;
  ug->SetPoints(points);

  vtkNew<vtkXMLUnstructuredGridWriter> writer;
  writer->SetFileName("output.vtu");
  writer->SetInputData(ug);
  writer->Write();

  return EXIT_SUCCESS;
}
