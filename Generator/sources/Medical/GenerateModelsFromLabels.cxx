//
// GenerateModelsFromLabels
//   Usage: GenerateModelsFromLabels InputVolume Startlabel Endlabel
//          where
//          InputVolume is a meta file containing a 3 volume of
//            discrete labels.
//          StartLabel is the first label to be processed
//          EndLabel is the last label to be processed
//          NOTE: There can be gaps in the labeling. If a label does
//          not exist in the volume, it will be skipped.
//
//
#include <vtkGeometryFilter.h>
#include <vtkImageAccumulate.h>
#include <vtkImageData.h>
#include <vtkMaskFields.h>
#include <vtkMetaImageReader.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkThreshold.h>
#include <vtkVersion.h>
#include <vtkWindowedSincPolyDataFilter.h>
#include <vtkXMLPolyDataWriter.h>

// vtkDiscreteFlyingEdges3D was introduced in VTK >= 8.2
#if VTK_MAJOR_VERSION >= 9 || (VTK_MAJOR_VERSION >= 8 && VTK_MINOR_VERSION >= 2)
#define USE_FLYING_EDGES
#else
#undef USE_FLYING_EDGES
#endif
// #undef USE_FLYING_EDGES

#ifdef USE_FLYING_EDGES
#include <vtkDiscreteFlyingEdges3D.h>
#else
#include <vtkDiscreteMarchingCubes.h>
#endif

#include <sstream>

int main(int argc, char* argv[])
{
  if (argc < 4)
  {
    std::cout
        << "Usage: " << argv[0]
        << " InputVolume StartLabel EndLabel  e.g. Frog/frogtissue.mhd 1 29"
        << std::endl;
    return EXIT_FAILURE;
  }

  // Create all of the classes we will need.
  vtkNew<vtkMetaImageReader> reader;
  vtkNew<vtkImageAccumulate> histogram;
#ifdef USE_FLYING_EDGES
  vtkNew<vtkDiscreteFlyingEdges3D> discreteCubes;
#else
  vtkNew<vtkDiscreteMarchingCubes> discreteCubes;
#endif
  vtkNew<vtkWindowedSincPolyDataFilter> smoother;
  vtkNew<vtkThreshold> selector;
  vtkNew<vtkMaskFields> scalarsOff;
  vtkNew<vtkGeometryFilter> geometry;
  vtkNew<vtkXMLPolyDataWriter> writer;

  // Define all of the variables
  unsigned int startLabel = atoi(argv[2]);
  unsigned int endLabel = atoi(argv[3]);
  std::string filePrefix = "Label";
  unsigned int smoothingIterations = 15;
  double passBand = 0.001;
  double featureAngle = 120.0;

  // Generate models from labels.
  // 1) Read the meta file
  // 2) Generate a histogram of the labels
  // 3) Generate models from the labeled volume
  // 4) Smooth the models
  // 5) Output each model into a separate file

  reader->SetFileName(argv[1]);

  histogram->SetInputConnection(reader->GetOutputPort());
  histogram->SetComponentExtent(0, endLabel, 0, 0, 0, 0);
  histogram->SetComponentOrigin(0, 0, 0);
  histogram->SetComponentSpacing(1, 1, 1);
  histogram->Update();

  discreteCubes->SetInputConnection(reader->GetOutputPort());
  discreteCubes->GenerateValues(endLabel - startLabel + 1, startLabel,
                                endLabel);

  smoother->SetInputConnection(discreteCubes->GetOutputPort());
  smoother->SetNumberOfIterations(smoothingIterations);
  smoother->BoundarySmoothingOff();
  smoother->FeatureEdgeSmoothingOff();
  smoother->SetFeatureAngle(featureAngle);
  smoother->SetPassBand(passBand);
  smoother->NonManifoldSmoothingOn();
  smoother->NormalizeCoordinatesOn();
  smoother->Update();

  selector->SetInputConnection(smoother->GetOutputPort());
#ifdef USE_FLYING_EDGES
  selector->SetInputArrayToProcess(0, 0, 0,
                                   vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                   vtkDataSetAttributes::SCALARS);
#else
  selector->SetInputArrayToProcess(0, 0, 0,
                                   vtkDataObject::FIELD_ASSOCIATION_CELLS,
                                   vtkDataSetAttributes::SCALARS);
#endif

  // Strip the scalars from the output
  scalarsOff->SetInputConnection(selector->GetOutputPort());
  scalarsOff->CopyAttributeOff(vtkMaskFields::POINT_DATA,
                               vtkDataSetAttributes::SCALARS);
  scalarsOff->CopyAttributeOff(vtkMaskFields::CELL_DATA,
                               vtkDataSetAttributes::SCALARS);

  geometry->SetInputConnection(scalarsOff->GetOutputPort());

  writer->SetInputConnection(geometry->GetOutputPort());

  for (unsigned int i = startLabel; i <= endLabel; i++)
  {
    // see if the label exists, if not skip it
    double frequency =
        histogram->GetOutput()->GetPointData()->GetScalars()->GetTuple1(i);
    if (frequency == 0.0)
    {
      continue;
    }

    // select the cells for a given label
    selector->SetLowerThreshold(i);
    selector->SetUpperThreshold(i);

    // output the polydata
    std::stringstream ss;
    ss << filePrefix << i << ".vtp";
    std::cout << argv[0] << " writing " << ss.str() << std::endl;

    writer->SetFileName(ss.str().c_str());
    writer->Write();
  }
  return EXIT_SUCCESS;
}
