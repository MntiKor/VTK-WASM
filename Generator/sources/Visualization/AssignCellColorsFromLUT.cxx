/*
Demonstrates how to assign colors to cells in a vtkPolyData structure using
 lookup tables.
Two techniques are demonstrated:
1) Using a lookup table of predefined colors.
2) Using a lookup table generated from a color transfer function.

The resultant display shows in the left-hand column, the cells in a plane
colored by the two lookup tables and in the right-hand column, the same
polydata that has been read in from a file demonstrating that the structures
are identical.

The top row of the display uses the color transfer function to create a
green to tan transition in a diverging color space.
Note that the central square is white indicating the midpoint.

The bottom row of the display uses a lookup table of predefined colors.
*/

#include <vtkActor.h>
#include <vtkCellData.h>
#include <vtkColorTransferFunction.h>
#include <vtkLookupTable.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPlaneSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkUnsignedCharArray.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>

#include <algorithm>
#include <iomanip>
#include <iostream>

template <typename T> void PrintColour(T& rgb)
{
  // Don't do this in real code! Range checking etc. is needed.
  for (size_t i = 0; i < 3; ++i)
  {
    if (i < 2)
    {
      std::cout << static_cast<double>(rgb[i]) << " ";
    }
    else
    {
      std::cout << static_cast<double>(rgb[i]);
    }
  }
}

//! Make a lookup table from a set of named colors.
/*
 * See: http://www.vtk.org/doc/nightly/html/classvtkColorTransferFunction.html
 */
void MakeLUT(size_t const& tableSize, vtkLookupTable* lut)
{
  vtkNew<vtkNamedColors> nc;

  lut->SetNumberOfTableValues(static_cast<vtkIdType>(tableSize));
  lut->Build();

  // Fill in a few known colors, the rest will be generated if needed.
  lut->SetTableValue(0, nc->GetColor4d("Black").GetData());
  lut->SetTableValue(1, nc->GetColor4d("Banana").GetData());
  lut->SetTableValue(2, nc->GetColor4d("Tomato").GetData());
  lut->SetTableValue(3, nc->GetColor4d("Wheat").GetData());
  lut->SetTableValue(4, nc->GetColor4d("Lavender").GetData());
  lut->SetTableValue(5, nc->GetColor4d("Flesh").GetData());
  lut->SetTableValue(6, nc->GetColor4d("Raspberry").GetData());
  lut->SetTableValue(7, nc->GetColor4d("Salmon").GetData());
  lut->SetTableValue(8, nc->GetColor4d("Mint").GetData());
  lut->SetTableValue(9, nc->GetColor4d("Peacock").GetData());
}

//! Use a color transfer Function to generate the colors in the lookup table.
void MakeLUTFromCTF(size_t const& tableSize, vtkLookupTable* lut)
{
  vtkNew<vtkColorTransferFunction> ctf;
  ctf->SetColorSpaceToDiverging();
  // Green to tan.
  ctf->AddRGBPoint(0.0, 0.085, 0.532, 0.201);
  ctf->AddRGBPoint(0.5, 0.865, 0.865, 0.865);
  ctf->AddRGBPoint(1.0, 0.677, 0.492, 0.093);

  lut->SetNumberOfTableValues(static_cast<vtkIdType>(tableSize));
  lut->Build();

  for (size_t i = 0; i < tableSize; ++i)
  {
    double* rgb;
    rgb = ctf->GetColor(static_cast<double>(i) / tableSize);
    lut->SetTableValue(static_cast<vtkIdType>(i), rgb);
  }
}

//! Create the cell data using the colors from the lookup table.
void MakeCellData(size_t const& tableSize, vtkLookupTable* lut,
                  vtkUnsignedCharArray* colors)
{
  for (size_t i = 1; i < tableSize; i++)
  {
    double rgb[3];
    unsigned char ucrgb[3];
    // Get the interpolated color.
    // Of course you can use any function whose range is [0...1]
    // to get the required color and assign it to a cell Id.
    // In this case we are just using the cell (Id + 1)/(tableSize - 1)
    // to get the interpolated color.
    lut->GetColor(static_cast<double>(i) / (tableSize - 1), rgb);
    for (size_t j = 0; j < 3; ++j)
    {
      ucrgb[j] = static_cast<unsigned char>(rgb[j] * 255);
    }
    colors->InsertNextTuple3(ucrgb[0], ucrgb[1], ucrgb[2]);
    // Print out what we have.
    std::cout << "(";
    PrintColour<double[3]>(rgb);
    std::cout << ") (";
    PrintColour<unsigned char[3]>(ucrgb);
    std::cout << ")" << std::endl;
  }
}

int main(int, char*[])
{
  vtkNew<vtkNamedColors> nc;

  // Provide some geometry.
  int resolution = 3;
  vtkNew<vtkPlaneSource> plane11;
  plane11->SetXResolution(resolution);
  plane11->SetYResolution(resolution);

  vtkNew<vtkPlaneSource> plane12;
  plane12->SetXResolution(resolution);
  plane12->SetYResolution(resolution);

  // Create a lookup table to map cell data to colors.
  vtkNew<vtkLookupTable> lut1;
  vtkNew<vtkLookupTable> lut2;
  int tableSize = std::max(resolution * resolution + 1, 10);

  // Force an update so we can set cell data.
  plane11->Update();
  plane12->Update();

  MakeLUT(tableSize, lut1);
  MakeLUTFromCTF(tableSize, lut2);

  vtkNew<vtkUnsignedCharArray> colorData1;
  colorData1->SetName("colors"); // Any name will work here.
  colorData1->SetNumberOfComponents(3);
  std::cout << "Using a lookup table from a set of named colors." << std::endl;
  MakeCellData(tableSize, lut1, colorData1);
  // Then use SetScalars() to add it to the vtkPolyData structure,
  // this will then be interpreted as a color table.
  plane11->GetOutput()->GetCellData()->SetScalars(colorData1);

  vtkNew<vtkUnsignedCharArray> colorData2;
  colorData2->SetName("colors");
  colorData2->SetNumberOfComponents(3);
  std::cout << "Using a lookup table created from a color transfer function."
            << std::endl;
  MakeCellData(tableSize, lut2, colorData2);
  plane12->GetOutput()->GetCellData()->SetScalars(colorData2);

  // Setup actor and mapper.
  vtkNew<vtkPolyDataMapper> mapper11;
  mapper11->SetInputConnection(plane11->GetOutputPort());
  // Now, instead of doing this:
  // mapper11->SetScalarRange(0, tableSize - 1);
  // mapper11->SetLookupTable(lut1);
  // We can just use the color data that we created from the lookup table and
  // assigned to the cells:
  mapper11->SetScalarModeToUseCellData();
  mapper11->Update();

  vtkNew<vtkPolyDataMapper> mapper12;
  mapper12->SetInputConnection(plane12->GetOutputPort());
  mapper12->SetScalarModeToUseCellData();
  mapper12->Update();

  vtkNew<vtkXMLPolyDataWriter> writer;
  writer->SetFileName("pdlut.vtp");
  writer->SetInputData(mapper11->GetInput());
  // This is set so we can see the data in a text editor.
  writer->SetDataModeToAscii();
  writer->Write();
  writer->SetFileName("pdctf.vtp");
  writer->SetInputData(mapper12->GetInput());
  writer->Write();

  vtkNew<vtkActor> actor11;
  actor11->SetMapper(mapper11);
  vtkNew<vtkActor> actor12;
  actor12->SetMapper(mapper12);

  // Let's read in the data we wrote out.
  vtkNew<vtkXMLPolyDataReader> reader1;
  reader1->SetFileName("pdlut.vtp");

  vtkNew<vtkXMLPolyDataReader> reader2;
  reader2->SetFileName("pdctf.vtp");

  vtkNew<vtkPolyDataMapper> mapper21;
  mapper21->SetInputConnection(reader1->GetOutputPort());
  mapper21->SetScalarModeToUseCellData();
  mapper21->Update();
  vtkNew<vtkActor> actor21;
  actor21->SetMapper(mapper11);

  vtkNew<vtkPolyDataMapper> mapper22;
  mapper22->SetInputConnection(reader2->GetOutputPort());
  mapper22->SetScalarModeToUseCellData();
  mapper22->Update();
  vtkNew<vtkActor> actor22;
  actor22->SetMapper(mapper22);

  // Define viewport ranges.
  // (xmin, ymin, xmax, ymax)
  double viewport11[4] = {0.0, 0.0, 0.5, 0.5};
  double viewport12[4] = {0.0, 0.5, 0.5, 1.0};
  double viewport21[4] = {0.5, 0.0, 1.0, 0.5};
  double viewport22[4] = {0.5, 0.5, 1.0, 1.0};

  // Set up the renderers.
  vtkNew<vtkRenderer> ren11;
  vtkNew<vtkRenderer> ren12;
  vtkNew<vtkRenderer> ren21;
  vtkNew<vtkRenderer> ren22;

  // Setup the render windows.
  vtkNew<vtkRenderWindow> renWin;
  renWin->SetSize(600, 600);
  renWin->SetWindowName("AssignCellColorsFromLUT");

  renWin->AddRenderer(ren11);
  renWin->AddRenderer(ren12);
  renWin->AddRenderer(ren21);
  renWin->AddRenderer(ren22);
  ren11->SetViewport(viewport11);
  ren12->SetViewport(viewport12);
  ren21->SetViewport(viewport21);
  ren22->SetViewport(viewport22);
  ren11->SetBackground(nc->GetColor3d("MidnightBlue").GetData());
  ren12->SetBackground(nc->GetColor3d("MidnightBlue").GetData());
  ren21->SetBackground(nc->GetColor3d("MidnightBlue").GetData());
  ren22->SetBackground(nc->GetColor3d("MidnightBlue").GetData());
  ren11->AddActor(actor11);
  ren12->AddActor(actor12);
  ren21->AddActor(actor21);
  ren22->AddActor(actor22);

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);
  renWin->Render();
  iren->Start();

  return EXIT_SUCCESS;
}
