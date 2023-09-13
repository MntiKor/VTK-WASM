#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkDICOMImageReader.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkImageData.h>
#include <vtkImageResample.h>
#include <vtkInteractorObserver.h>
#include <vtkMetaImageReader.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkXMLImageDataReader.h>

#include <iostream>
#include <string>

#define VTI_FILETYPE 1
#define MHA_FILETYPE 2

namespace {
void PrintUsage()
{
  std::cout << "Usage: " << std::endl;
  std::cout << std::endl;
  std::cout << "  FixedPointVolumeRayCastMapperCT <options>" << std::endl;
  std::cout << std::endl;
  std::cout << "where options may include: " << std::endl;
  std::cout << std::endl;
  std::cout << "  -DICOM <directory>" << std::endl;
  std::cout << "  -VTI <filename>" << std::endl;
  std::cout << "  -MHA <filename>" << std::endl;
  std::cout << "  -DependentComponents" << std::endl;
  std::cout << "  -Clip" << std::endl;
  std::cout << "  -MIP <window> <level>" << std::endl;
  std::cout << "  -CompositeRamp <window> <level>" << std::endl;
  std::cout << "  -CompositeShadeRamp <window> <level>" << std::endl;
  std::cout << "  -CT_Skin" << std::endl;
  std::cout << "  -CT_Bone" << std::endl;
  std::cout << "  -CT_Muscle" << std::endl;
  std::cout << "  -FrameRate <rate>" << std::endl;
  std::cout << "  -DataReduction <factor>" << std::endl;
  std::cout << std::endl;
  std::cout
      << "You must use either the -DICOM option to specify the directory where"
      << std::endl;
  std::cout
      << "the data is located or the -VTI or -MHA option to specify the path "
         "of a .vti file."
      << std::endl;
  std::cout << std::endl;
  std::cout << "By default, the program assumes that the file has independent "
               "components,"
            << std::endl;
  std::cout
      << "use -DependentComponents to specify that the file has dependent "
         "components."
      << std::endl;
  std::cout << std::endl;
  std::cout << "Use the -Clip option to display a cube widget for clipping the "
               "volume."
            << std::endl;
  std::cout
      << "Use the -FrameRate option with a desired frame rate (in frames per "
         "second)"
      << std::endl;
  std::cout << "which will control the interactive rendering rate."
            << std::endl;
  std::cout
      << "Use the -DataReduction option with a reduction factor (greater than "
         "zero and"
      << std::endl;
  std::cout << "less than one) to reduce the data before rendering."
            << std::endl;
  std::cout << "Use one of the remaining options to specify the blend function"
            << std::endl;
  std::cout
      << "and transfer functions. The -MIP option utilizes a maximum intensity"
      << std::endl;
  std::cout << "projection method, while the others utilize compositing. The"
            << std::endl;
  std::cout << "-CompositeRamp option is unshaded compositing, while the other"
            << std::endl;
  std::cout << "compositing options employ shading." << std::endl;
  std::cout << std::endl;
  std::cout << "Note: MIP, CompositeRamp, CompositeShadeRamp, CT_Skin, CT_Bone,"
            << std::endl;
  std::cout
      << "and CT_Muscle are appropriate for DICOM data. MIP, CompositeRamp,"
      << std::endl;
  std::cout << "and RGB_Composite are appropriate for RGB data." << std::endl;
  std::cout << std::endl;
  std::cout
      << "Example: FixedPointVolumeRayCastMapperCT -DICOM CTNeck -MIP 4096 1024"
      << std::endl;
  std::cout << "e.g. -MHA /FullHead.mhd -CT_Bone" << std::endl;
  std::cout << std::endl;
}
} // namespace

int main(int argc, char* argv[])
{
  // Parse the parameters

  int count = 1;
  char* dirname = NULL;
  double opacityWindow = 4096;
  double opacityLevel = 2048;
  int blendType = 0;
  int clip = 0;
  double reductionFactor = 1.0;
  double frameRate = 10.0;
  char* fileName = 0;
  int fileType = 0;

  bool independentComponents = true;

  while (count < argc)
  {
    if (!strcmp(argv[count], "?"))
    {
      PrintUsage();
      exit(EXIT_SUCCESS);
    }
    else if (!strcmp(argv[count], "-DICOM"))
    {
      size_t size = strlen(argv[count + 1]) + 1;
      dirname = new char[size];
      snprintf(dirname, size, "%s", argv[count + 1]);
      count += 2;
    }
    else if (!strcmp(argv[count], "-VTI"))
    {
      size_t size = strlen(argv[count + 1]) + 1;
      fileName = new char[size];
      fileType = VTI_FILETYPE;
      snprintf(fileName, size, "%s", argv[count + 1]);
      count += 2;
    }
    else if (!strcmp(argv[count], "-MHA"))
    {
      size_t size = strlen(argv[count + 1]) + 1;
      fileName = new char[size];
      fileType = MHA_FILETYPE;
      snprintf(fileName, size, "%s", argv[count + 1]);
      count += 2;
    }
    else if (!strcmp(argv[count], "-Clip"))
    {
      clip = 1;
      count++;
    }
    else if (!strcmp(argv[count], "-MIP"))
    {
      opacityWindow = atof(argv[count + 1]);
      opacityLevel = atof(argv[count + 2]);
      blendType = 0;
      count += 3;
    }
    else if (!strcmp(argv[count], "-CompositeRamp"))
    {
      opacityWindow = atof(argv[count + 1]);
      opacityLevel = atof(argv[count + 2]);
      blendType = 1;
      count += 3;
    }
    else if (!strcmp(argv[count], "-CompositeShadeRamp"))
    {
      opacityWindow = atof(argv[count + 1]);
      opacityLevel = atof(argv[count + 2]);
      blendType = 2;
      count += 3;
    }
    else if (!strcmp(argv[count], "-CT_Skin"))
    {
      blendType = 3;
      count += 1;
    }
    else if (!strcmp(argv[count], "-CT_Bone"))
    {
      blendType = 4;
      count += 1;
    }
    else if (!strcmp(argv[count], "-CT_Muscle"))
    {
      blendType = 5;
      count += 1;
    }
    else if (!strcmp(argv[count], "-RGB_Composite"))
    {
      blendType = 6;
      count += 1;
    }
    else if (!strcmp(argv[count], "-FrameRate"))
    {
      frameRate = atof(argv[count + 1]);
      if (frameRate < 0.01 || frameRate > 60.0)
      {
        std::cout << "Invalid frame rate - use a number between 0.01 and 60.0"
                  << std::endl;
        std::cout << "Using default frame rate of 10 frames per second."
                  << std::endl;
        frameRate = 10.0;
      }
      count += 2;
    }
    else if (!strcmp(argv[count], "-ReductionFactor"))
    {
      reductionFactor = atof(argv[count + 1]);
      if (reductionFactor <= 0.0 || reductionFactor >= 1.0)
      {
        std::cout << "Invalid reduction factor - use a number between 0 and 1 "
                     "(exclusive)"
                  << std::endl;
        std::cout << "Using the default of no reduction." << std::endl;
        reductionFactor = 1.0;
      }
      count += 2;
    }
    else if (!strcmp(argv[count], "-DependentComponents"))
    {
      independentComponents = false;
      count += 1;
    }
    else
    {
      std::cout << "Unrecognized option: " << argv[count] << std::endl;
      std::cout << std::endl;
      PrintUsage();
      exit(EXIT_FAILURE);
    }
  }

  if (!dirname && !fileName)
  {
    std::cout
        << "Error: you must specify a directory of DICOM data or a .vti file "
           "or a .mha!"
        << std::endl;
    std::cout << std::endl;
    PrintUsage();
    exit(EXIT_FAILURE);
  }

  // Create the renderer, render window and interactor
  vtkNew<vtkNamedColors> colors;
  vtkNew<vtkRenderer> renderer;
  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(renderer);

  // Connect it all. Note that funny arithematic on the
  // SetDesiredUpdateRate - the vtkRenderWindow divides it
  // allocated time across all renderers, and the renderer
  // divides it time across all props. If clip is
  // true then there are two props
  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);
  iren->SetDesiredUpdateRate(frameRate / (1 + clip));

  iren->GetInteractorStyle()->SetDefaultRenderer(renderer);

  // Read the data
  vtkSmartPointer<vtkAlgorithm> reader;
  vtkSmartPointer<vtkImageData> input;
  if (dirname)
  {
    vtkNew<vtkDICOMImageReader> dicomReader;
    dicomReader->SetDirectoryName(dirname);
    dicomReader->Update();
    input = dicomReader->GetOutput();
    reader = dicomReader;
  }
  else if (fileType == VTI_FILETYPE)
  {
    vtkNew<vtkXMLImageDataReader> xmlReader;
    xmlReader->SetFileName(fileName);
    xmlReader->Update();
    input = xmlReader->GetOutput();
    reader = xmlReader;
  }
  else if (fileType == MHA_FILETYPE)
  {
    vtkNew<vtkMetaImageReader> metaReader;
    metaReader->SetFileName(fileName);
    metaReader->Update();
    input = metaReader->GetOutput();
    reader = metaReader;
  }
  else
  {
    std::cout << "Error! Not VTI or MHA!" << std::endl;
    exit(EXIT_FAILURE);
  }

  // Verify that we actually have a volume
  int dim[3];
  input->GetDimensions(dim);

  if (dim[0] < 2 || dim[1] < 2 || dim[2] < 2)
  {
    std::cout << "Error loading data!" << std::endl;
    exit(EXIT_FAILURE);
  }

  vtkNew<vtkImageResample> resample;
  if (reductionFactor < 1.0)
  {
    resample->SetInputConnection(reader->GetOutputPort());
    resample->SetAxisMagnificationFactor(0, reductionFactor);
    resample->SetAxisMagnificationFactor(1, reductionFactor);
    resample->SetAxisMagnificationFactor(2, reductionFactor);
  }

  // Create our volume and mapper
  vtkNew<vtkVolume> volume;
  vtkNew<vtkFixedPointVolumeRayCastMapper> mapper;

  if (reductionFactor < 1.0)
  {
    mapper->SetInputConnection(resample->GetOutputPort());
  }
  else
  {
    mapper->SetInputConnection(reader->GetOutputPort());
  }

  // Set the sample distance on the ray to be 1/2 the average spacing
  double spacing[3];
  if (reductionFactor < 1.0)
  {
    resample->GetOutput()->GetSpacing(spacing);
  }
  else
  {
    input->GetSpacing(spacing);
  }

  //  mapper->SetSampleDistance( (spacing[0]+spacing[1]+spacing[2])/6.0 );
  //  mapper->SetMaximumImageSampleDistance(10.0);

  // Create our transfer function
  vtkNew<vtkColorTransferFunction> colorFun;
  vtkNew<vtkPiecewiseFunction> opacityFun;
  // Create the property and attach the transfer functions
  vtkNew<vtkVolumeProperty> property;
  property->SetIndependentComponents(independentComponents);
  property->SetColor(colorFun);
  property->SetScalarOpacity(opacityFun);
  property->SetInterpolationTypeToLinear();

  // connect up the volume to the property and the mapper
  volume->SetProperty(property);
  volume->SetMapper(mapper);

  // Depending on the blend type selected as a command line option,
  // adjust the transfer function
  switch (blendType)
  {
  // MIP
  // Create an opacity ramp from the window and level values.
  // Color is white. Blending is MIP.
  case 0:
    colorFun->AddRGBSegment(0.0, 1.0, 1.0, 1.0, 255.0, 1.0, 1.0, 1.0);
    opacityFun->AddSegment(opacityLevel - 0.5 * opacityWindow, 0.0,
                           opacityLevel + 0.5 * opacityWindow, 1.0);
    mapper->SetBlendModeToMaximumIntensity();
    break;

  // CompositeRamp
  // Create a ramp from the window and level values. Use compositing
  // without shading. Color is a ramp from black to white.
  case 1:
    colorFun->AddRGBSegment(opacityLevel - 0.5 * opacityWindow, 0.0, 0.0, 0.0,
                            opacityLevel + 0.5 * opacityWindow, 1.0, 1.0, 1.0);
    opacityFun->AddSegment(opacityLevel - 0.5 * opacityWindow, 0.0,
                           opacityLevel + 0.5 * opacityWindow, 1.0);
    mapper->SetBlendModeToComposite();
    property->ShadeOff();
    break;

  // CompositeShadeRamp
  // Create a ramp from the window and level values. Use compositing
  // with shading. Color is white.
  case 2:
    colorFun->AddRGBSegment(0.0, 1.0, 1.0, 1.0, 255.0, 1.0, 1.0, 1.0);
    opacityFun->AddSegment(opacityLevel - 0.5 * opacityWindow, 0.0,
                           opacityLevel + 0.5 * opacityWindow, 1.0);
    mapper->SetBlendModeToComposite();
    property->ShadeOn();
    break;

  // CT_Skin
  // Use compositing and functions set to highlight skin in CT data
  // Not for use on RGB data
  case 3:
    colorFun->AddRGBPoint(-3024, 0, 0, 0, 0.5, 0.0);
    colorFun->AddRGBPoint(-1000, .62, .36, .18, 0.5, 0.0);
    colorFun->AddRGBPoint(-500, .88, .60, .29, 0.33, 0.45);
    colorFun->AddRGBPoint(3071, .83, .66, 1, 0.5, 0.0);

    opacityFun->AddPoint(-3024, 0, 0.5, 0.0);
    opacityFun->AddPoint(-1000, 0, 0.5, 0.0);
    opacityFun->AddPoint(-500, 1.0, 0.33, 0.45);
    opacityFun->AddPoint(3071, 1.0, 0.5, 0.0);

    mapper->SetBlendModeToComposite();
    property->ShadeOn();
    property->SetAmbient(0.1);
    property->SetDiffuse(0.9);
    property->SetSpecular(0.2);
    property->SetSpecularPower(10.0);
    property->SetScalarOpacityUnitDistance(0.8919);
    break;

  // CT_Bone
  // Use compositing and functions set to highlight bone in CT data
  // Not for use on RGB data
  case 4:
    colorFun->AddRGBPoint(-3024, 0, 0, 0, 0.5, 0.0);
    colorFun->AddRGBPoint(-16, 0.73, 0.25, 0.30, 0.49, .61);
    colorFun->AddRGBPoint(641, .90, .82, .56, .5, 0.0);
    colorFun->AddRGBPoint(3071, 1, 1, 1, .5, 0.0);

    opacityFun->AddPoint(-3024, 0, 0.5, 0.0);
    opacityFun->AddPoint(-16, 0, .49, .61);
    opacityFun->AddPoint(641, .72, .5, 0.0);
    opacityFun->AddPoint(3071, .71, 0.5, 0.0);

    mapper->SetBlendModeToComposite();
    property->ShadeOn();
    property->SetAmbient(0.1);
    property->SetDiffuse(0.9);
    property->SetSpecular(0.2);
    property->SetSpecularPower(10.0);
    property->SetScalarOpacityUnitDistance(0.8919);
    break;

  // CT_Muscle
  // Use compositing and functions set to highlight muscle in CT data
  // Not for use on RGB data
  case 5:
    colorFun->AddRGBPoint(-3024, 0, 0, 0, 0.5, 0.0);
    colorFun->AddRGBPoint(-155, .55, .25, .15, 0.5, .92);
    colorFun->AddRGBPoint(217, .88, .60, .29, 0.33, 0.45);
    colorFun->AddRGBPoint(420, 1, .94, .95, 0.5, 0.0);
    colorFun->AddRGBPoint(3071, .83, .66, 1, 0.5, 0.0);

    opacityFun->AddPoint(-3024, 0, 0.5, 0.0);
    opacityFun->AddPoint(-155, 0, 0.5, 0.92);
    opacityFun->AddPoint(217, .68, 0.33, 0.45);
    opacityFun->AddPoint(420, .83, 0.5, 0.0);
    opacityFun->AddPoint(3071, .80, 0.5, 0.0);

    mapper->SetBlendModeToComposite();
    property->ShadeOn();
    property->SetAmbient(0.1);
    property->SetDiffuse(0.9);
    property->SetSpecular(0.2);
    property->SetSpecularPower(10.0);
    property->SetScalarOpacityUnitDistance(0.8919);
    break;

  // RGB_Composite
  // Use compositing and functions set to highlight red/green/blue regions
  // in RGB data. Not for use on single component data
  case 6:
    opacityFun->AddPoint(0, 0.0);
    opacityFun->AddPoint(5.0, 0.0);
    opacityFun->AddPoint(30.0, 0.05);
    opacityFun->AddPoint(31.0, 0.0);
    opacityFun->AddPoint(90.0, 0.0);
    opacityFun->AddPoint(100.0, 0.3);
    opacityFun->AddPoint(110.0, 0.0);
    opacityFun->AddPoint(190.0, 0.0);
    opacityFun->AddPoint(200.0, 0.4);
    opacityFun->AddPoint(210.0, 0.0);
    opacityFun->AddPoint(245.0, 0.0);
    opacityFun->AddPoint(255.0, 0.5);

    mapper->SetBlendModeToComposite();
    property->ShadeOff();
    property->SetScalarOpacityUnitDistance(1.0);
    break;
  default:
    vtkGenericWarningMacro("Unknown blend type.");
    break;
  }

  // Set the default window size
  renWin->SetSize(600, 600);
  renWin->SetWindowName("FixedPointVolumeRayCastMapperCT");
  renWin->Render();

  // Add the volume to the scene
  renderer->AddVolume(volume);

  renderer->ResetCamera();
  renderer->SetBackground(colors->GetColor3d("SlateGray").GetData());

  auto camera = renderer->GetActiveCamera();
  camera->SetPosition(56.8656, -297.084, 78.913);
  camera->SetFocalPoint(109.139, 120.604, 63.5486);
  camera->SetViewUp(-0.00782421, -0.0357807, -0.999329);
  camera->SetDistance(421.227);
  camera->SetClippingRange(146.564, 767.987);

  // interact with data
  renWin->Render();

  iren->Start();

  return EXIT_SUCCESS;
}
