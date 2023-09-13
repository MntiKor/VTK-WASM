// .NAME Generate2DAMRDataSetWithPulse.cxx -- Generates sample 2-D AMR dataset
//
// .SECTION Description
//  This utility code generates a simple 2D AMR dataset with a gaussian
//  pulse at the center. The resulting AMR dataset is written using the
//  vtkXMLHierarchicalBoxDataSetWriter.

#include <cmath>
#include <iostream>
#include <sstream>

#include <vtkAMRBox.h>
#include <vtkAMRUtilities.h>
#include <vtkCell.h>
#include <vtkCellData.h>
#include <vtkCompositeDataWriter.h>
#include <vtkDoubleArray.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkNew.h>
#include <vtkOverlappingAMR.h>
#include <vtkSmartPointer.h>
#include <vtkUniformGrid.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkXMLMultiBlockDataWriter.h>

namespace AMRCommon {
void WriteUniformGrid(vtkUniformGrid* g, const std::string& prefix);
void WriteAMRData(vtkOverlappingAMR* amrData, const std::string& prefix);
void WriteMultiBlockData(vtkMultiBlockDataSet* mbds, const std::string& prefix);
vtkSmartPointer<vtkUniformGrid> GetGrid(double* origin, double* h, int* ndim);
void ComputeCellCenter(vtkUniformGrid* grid, const int cellIdx, double c[3]);
} // namespace AMRCommon

namespace {
struct PulseAttributes
{
  double origin[3]; // xyz for the center of the pulse
  double width[3];  // the width of the pulse
  double amplitude; // the amplitude of the pulse
} Pulse;
//
// Function prototype declarations
//

// Description:
// Sets the pulse attributes
void SetPulse();

// Description:
// Constructs the vtkOverlappingAMR.
vtkSmartPointer<vtkOverlappingAMR> GetAMRDataSet();

// Description:
// Attaches the pulse to the given grid.
void AttachPulseToGrid(vtkUniformGrid* grid);
} // namespace

//
// Program main
//
int main(int, char*[])
{
  // STEP 0: Initialize gaussian pulse parameters
  SetPulse();

  // STEP 1: Get the AMR dataset
  auto amrDataSet = GetAMRDataSet();
  AMRCommon::WriteAMRData(amrDataSet, "Gaussian2D");
  return EXIT_SUCCESS;
}

namespace {
//=============================================================================
//                    Function Prototype Implementation
//=============================================================================

void SetPulse()
{
  Pulse.origin[0] = Pulse.origin[1] = Pulse.origin[2] = -1.0;
  Pulse.width[0] = Pulse.width[1] = Pulse.width[2] = 6.0;
  Pulse.amplitude = 0.0001;
}

//------------------------------------------------------------------------------
void AttachPulseToGrid(vtkUniformGrid* grid)
{
  vtkNew<vtkDoubleArray> xyz;
  xyz->SetName("GaussianPulse");
  xyz->SetNumberOfComponents(1);
  xyz->SetNumberOfTuples(grid->GetNumberOfCells());

  for (int cellIdx = 0; cellIdx < grid->GetNumberOfCells(); ++cellIdx)
  {
    double center[3];
    AMRCommon::ComputeCellCenter(grid, cellIdx, center);

    auto r = 0.0;
    for (auto i = 0; i < 2; ++i)
    {
      auto dx = center[i] - Pulse.origin[i];
      r += (dx * dx) / (Pulse.width[i] * Pulse.width[i]);
    }
    auto f = Pulse.amplitude * std::exp(-r);

    xyz->SetTuple1(cellIdx, f);
  } // END for all cells

  grid->GetCellData()->AddArray(xyz);
}

//------------------------------------------------------------------------------
vtkSmartPointer<vtkOverlappingAMR> GetAMRDataSet()
{
  int NumLevels = 2;
  int BlocksPerLevel[2] = {1, 2};
  double origin[3] = {-2.0, -2.0, 0.0};

  vtkNew<vtkOverlappingAMR> data;
  data->Initialize(NumLevels, BlocksPerLevel);
  data->SetOrigin(origin);
  data->SetGridDescription(VTK_XY_PLANE);

  double h[3] = {1.0, 1.0, 1.0};
  int ndim[3] = {6, 5, 1};

  // Root Block -- Block 0,0
  auto blockId = 0;
  auto level = 0;
  auto root = AMRCommon::GetGrid(origin, h, ndim);
  vtkAMRBox box(origin, ndim, h, data->GetOrigin(), data->GetGridDescription());
  AttachPulseToGrid(root);

  data->SetSpacing(level, h);
  data->SetAMRBox(level, blockId, box);
  data->SetDataSet(level, blockId, root);

  // Block 1,0
  ndim[0] = ndim[1] = 9;
  ndim[2] = 1;
  h[0] = h[1] = h[2] = 0.25;
  origin[0] = origin[1] = -2.0;
  origin[2] = 0.0;
  blockId = 0;
  level = 1;
  auto grid1 = AMRCommon::GetGrid(origin, h, ndim);
  vtkAMRBox box1(origin, ndim, h, data->GetOrigin(),
                 data->GetGridDescription());
  AttachPulseToGrid(grid1);

  data->SetSpacing(level, h);
  data->SetAMRBox(level, blockId, box1);
  data->SetDataSet(level, blockId, grid1);

  // Block 1,1
  ndim[0] = ndim[1] = 9;
  ndim[2] = 1;
  h[0] = h[1] = h[2] = 0.25;
  origin[0] = 1.0;
  origin[1] = origin[2] = 0.0;
  blockId = 1;
  level = 1;
  auto grid3 = AMRCommon::GetGrid(origin, h, ndim);
  vtkAMRBox box3(origin, ndim, h, data->GetOrigin(),
                 data->GetGridDescription());

  AttachPulseToGrid(grid3);
  data->SetSpacing(level, h);
  data->SetAMRBox(level, blockId, box3);
  data->SetDataSet(level, blockId, grid3);

  vtkAMRUtilities::BlankCells(data);
  data->Audit();
  return (data);
}
} // namespace

namespace AMRCommon {

//------------------------------------------------------------------------------
// Description:
// Writes a uniform grid as a structure grid
void WriteUniformGrid(vtkUniformGrid* g, const std::string& prefix)
{
  vtkNew<vtkXMLImageDataWriter> imgWriter;

  std::ostringstream oss;
  oss << prefix << "." << imgWriter->GetDefaultFileExtension();
  imgWriter->SetFileName(oss.str().c_str());
  imgWriter->SetInputData(g);
  imgWriter->Write();
}

//------------------------------------------------------------------------------
// Description:
// Writes the given AMR dataset to a *.vth file with the given prefix.
void WriteAMRData(vtkOverlappingAMR* amrData, const std::string& prefix)
{
  vtkNew<vtkCompositeDataWriter> writer;

  std::ostringstream oss;
  oss << prefix << ".vthb";
  writer->SetFileName(oss.str().c_str());
  writer->SetInputData(amrData);
  writer->Write();
}

//------------------------------------------------------------------------------
// Description:
// Writes the given multi-block data to an XML file with the prescribed prefix
void WriteMultiBlockData(vtkMultiBlockDataSet* mbds, const std::string& prefix)
{
  // Sanity check
  vtkNew<vtkXMLMultiBlockDataWriter> writer;

  std::ostringstream oss;
  oss.str("");
  oss.clear();
  oss << prefix << "." << writer->GetDefaultFileExtension();
  writer->SetFileName(oss.str().c_str());
  writer->SetInputData(mbds);
  writer->Write();
}

//------------------------------------------------------------------------------
// Constructs a uniform grid instance given the prescribed
// origin, grid spacing and dimensions.
vtkSmartPointer<vtkUniformGrid> GetGrid(double* origin, double* h, int* ndim)
{
  vtkNew<vtkUniformGrid> grd;
  grd->Initialize();
  grd->SetOrigin(origin);
  grd->SetSpacing(h);
  grd->SetDimensions(ndim);
  return grd;
}

//------------------------------------------------------------------------------
// Computes the cell center for the cell corresponding to cellIdx w.r.t.
// the given grid. The cell center is stored in the supplied buffer c.
void ComputeCellCenter(vtkUniformGrid* grid, const int cellIdx, double c[3])
{
  vtkCell* myCell = grid->GetCell(cellIdx);

  double pCenter[3];
  double* weights = new double[myCell->GetNumberOfPoints()];
  int subId = myCell->GetParametricCenter(pCenter);
  myCell->EvaluateLocation(subId, pCenter, c, weights);
  delete[] weights;
}

} // namespace AMRCommon
