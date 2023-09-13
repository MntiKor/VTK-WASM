#include <vtkActor.h>
#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOpenGLPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkShaderProgram.h>
#include <vtkSmartPointer.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>
#include <vtkTriangleMeshPointNormals.h>
#include <vtkVersion.h>

#include <vtkBYUReader.h>
#include <vtkOBJReader.h>
#include <vtkPLYReader.h>
#include <vtkPolyDataReader.h>
#include <vtkSTLReader.h>
#include <vtkSphereSource.h>
#include <vtkXMLPolyDataReader.h>
#include <vtksys/SystemTools.hxx>

#if VTK_VERSION_NUMBER >= 89000000000ULL
#define USE_SHADER_PROPERTIES 1
#include <vtkShaderProperty.h>
#endif

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {
// -----------------------------------------------------------------------
// Update a uniform in the shader for each render. We do this with a
// callback for the UpdateShaderEvent
class vtkShaderCallback : public vtkCommand
{
public:
  static vtkShaderCallback* New()
  {
    return new vtkShaderCallback;
  }
  vtkRenderer* Renderer;
  float k;
  void Execute(vtkObject*, unsigned long, void* calldata) override
  {
    vtkShaderProgram* program = reinterpret_cast<vtkShaderProgram*>(calldata);
    if (program)
    {
      program->SetUniformf("k", k);
    }
  }
  void Print(std::ostream& os)
  {
    os << "k: " << k << std::endl;
  }

  vtkShaderCallback()
  {
    this->Renderer = nullptr;
    this->k = 5;
  }
};
} // namespace

namespace {
vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName);
}

//----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cout << "Usage: " << argv[0] << " PerlnNoise.glsl "
              << "[polydataFile] "
              << "[k(5)]" << std::endl;
    return EXIT_FAILURE;
  }
  vtkNew<vtkNamedColors> colors;

  vtkNew<vtkRenderWindow> renderWindow;

  // Create one text property for all
  vtkNew<vtkTextProperty> textProperty;
  textProperty->SetFontSize(20);
  //  textProperty->SetJustificationToCentered();
  textProperty->SetColor(0.3, 0.3, 0.3);

  std::vector<vtkSmartPointer<vtkRenderer>> renderers;
  for (int i = 2; i < argc; ++i)
  {
    std::cout << "Reading file: " << argv[i] << std::endl;
    auto polyData = ReadPolyData(argv[i]);
    std::ifstream shaderFile(argv[1]);
    std::ostringstream shaderCode;
    shaderCode << shaderFile.rdbuf();
    shaderFile.close();

    // Create a transform to rescale model
    double center[3];
    polyData->GetCenter(center);
    double bounds[6];
    polyData->GetBounds(bounds);
    double maxBound =
        std::max(std::max(bounds[1] - bounds[0], bounds[3] - bounds[2]),
                 bounds[5] - bounds[4]);

    // Create textActors
    vtkNew<vtkTextMapper> textMapper;
    textMapper->SetTextProperty(textProperty);
    textMapper->SetInput(vtksys::SystemTools::GetFilenameName(argv[i]).c_str());

    vtkNew<vtkActor2D> textActor;
    textActor->SetMapper(textMapper);
    textActor->SetPosition(20, 20);

    vtkNew<vtkActor> actor;
    vtkNew<vtkRenderer> renderer;
    vtkNew<vtkOpenGLPolyDataMapper> mapper;
    renderer->SetBackground(colors->GetColor3d("Burlywood").GetData());

    renderers.push_back(renderer);
    renderWindow->AddRenderer(renderer);
    renderer->AddActor(actor);
    renderer->AddActor(textActor);

    // Rescale polydata to [-1,1]
    vtkNew<vtkTransform> userTransform;
    userTransform->Translate(-center[0], -center[1], -center[2]);
    userTransform->Scale(1.0 / maxBound, 1.0 / maxBound, 1.0 / maxBound);
    vtkNew<vtkTransformPolyDataFilter> transform;
    transform->SetTransform(userTransform);
    transform->SetInputData(polyData);

    vtkNew<vtkTriangleFilter> triangles;
    triangles->SetInputConnection(transform->GetOutputPort());

    vtkNew<vtkTriangleMeshPointNormals> norms;
    norms->SetInputConnection(triangles->GetOutputPort());

    mapper->SetInputConnection(norms->GetOutputPort());
    mapper->ScalarVisibilityOff();

    actor->SetMapper(mapper);
    actor->GetProperty()->SetAmbientColor(0.2, 0.2, 0.2);
    actor->GetProperty()->SetDiffuseColor(1.0, 1.0, 1.0);
    actor->GetProperty()->SetSpecularColor(1.0, 1.0, 1.0);
    actor->GetProperty()->SetSpecular(0.5);
    actor->GetProperty()->SetDiffuse(0.7);
    actor->GetProperty()->SetAmbient(0.1);
    actor->GetProperty()->SetSpecularPower(100.0);
    actor->GetProperty()->SetOpacity(1.0);

    renderer->ResetCamera();
    renderer->GetActiveCamera()->Zoom(1.2);

    // Modify the vertex shader to pass the position of the vertex
#if USE_SHADER_PROPERTIES
    vtkShaderProperty* sp = actor->GetShaderProperty();
    sp->AddVertexShaderReplacement(
        "//VTK::Normal::Dec",  // replace the normal block
        true,                  // before the standard replacements
        "//VTK::Normal::Dec\n" // we still want the default
        "  out vec4 myVertexMC;\n",
        false // only do it once
    );
#else
    mapper->AddShaderReplacement(
        vtkShader::Vertex,
        "//VTK::Normal::Dec",  // replace the normal block
        true,                  // before the standard replacements
        "//VTK::Normal::Dec\n" // we still want the default
        "  out vec4 myVertexMC;\n",
        false // only do it once
    );
#endif
#if USE_SHADER_PROPERTIES
    sp->AddVertexShaderReplacement(
        "//VTK::Normal::Impl",  // replace the normal block
        true,                   // before the standard replacements
        "//VTK::Normal::Impl\n" // we still want the default
        "  myVertexMC = vertexMC;\n",
        false // only do it once
    );

#else
    mapper->AddShaderReplacement(
        vtkShader::Vertex,
        "//VTK::Normal::Impl",  // replace the normal block
        true,                   // before the standard replacements
        "//VTK::Normal::Impl\n" // we still want the default
        "  myVertexMC = vertexMC;\n",
        false // only do it once
    );

#endif
    // Add the code to generate noise
    // These functions need to be defined outside of main. Use the System::Dec
    // to declare and implement
#if USE_SHADER_PROPERTIES
    sp->AddFragmentShaderReplacement("//VTK::System::Dec",
                                     false, // before the standard replacements
                                     shaderCode.str(),
                                     false // only do it once
    );
#else
    mapper->AddShaderReplacement(vtkShader::Fragment, "//VTK::System::Dec",
                                 false, // before the standard replacements
                                 shaderCode.str(),
                                 false // only do it once
    );
#endif

    // Define varying and uniforms for the fragment shader here
#if USE_SHADER_PROPERTIES
    sp->AddFragmentShaderReplacement(
        "//VTK::Normal::Dec",  // replace the normal block
        true,                  // before the standard replacements
        "//VTK::Normal::Dec\n" // we still want the default
        "  varying vec4 myVertexMC;\n"
        "  uniform float k = 1.0;\n",
        false // only do it once
    );
#else
    mapper->AddShaderReplacement(
        vtkShader::Fragment,   // in the fragment shader
        "//VTK::Normal::Dec",  // replace the normal block
        true,                  // before the standard replacements
        "//VTK::Normal::Dec\n" // we still want the default
        "  varying vec4 myVertexMC;\n"
        "  uniform float k = 1.0;\n",
        false // only do it once
    );
#endif

#if USE_SHADER_PROPERTIES
    sp->AddFragmentShaderReplacement(
        "//VTK::Light::Impl",  // replace the light block
        false,                 // after the standard replacements
        "//VTK::Light::Impl\n" // we still want the default calc
        "#define pnoise(x) ((noise(x) + 1.0) / 2.0)\n"
        "  vec3 noisyColor;\n"
        "  noisyColor.r = noise(k * 10.0 * myVertexMC);\n"
        "  noisyColor.g = noise(k * 11.0 * myVertexMC);\n"
        "  noisyColor.b = noise(k * 12.0 * myVertexMC);\n"
        "  /* map ranges of noise values into different colors */\n"
        "  int i;\n"
        "  float lowerValue = .3;\n"
        "  float upperValue = .6;\n"
        "  for ( i=0; i<3; i+=1)\n"
        "  {\n"
        "    noisyColor[i] = (noisyColor[i] + 1.0) / 2.0;\n"
        "    if (noisyColor[i] < lowerValue) \n"
        "    {\n"
        "      noisyColor[i] = lowerValue;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "      if (noisyColor[i] < upperValue)\n"
        "      {\n"
        "        noisyColor[i] = upperValue;\n"
        "      }\n"
        "      else\n"
        "      {\n"
        "        noisyColor[i] = 1.0;\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "  fragOutput0.rgb = opacity * vec3(ambientColor + noisyColor * "
        "diffuse + specular);\n"
        "  fragOutput0.a = opacity;\n",
        false // only do it once
    );
#else
    mapper->AddShaderReplacement(
        vtkShader::Fragment,   // in the fragment shader
        "//VTK::Light::Impl",  // replace the light block
        false,                 // after the standard replacements
        "//VTK::Light::Impl\n" // we still want the default calc
        "#define pnoise(x) ((noise(x) + 1.0) / 2.0)\n"
        "  vec3 noisyColor;\n"
        "  noisyColor.r = noise(k * 10.0 * myVertexMC);\n"
        "  noisyColor.g = noise(k * 11.0 * myVertexMC);\n"
        "  noisyColor.b = noise(k * 12.0 * myVertexMC);\n"
        "  /* map ranges of noise values into different colors */\n"
        "  int i;\n"
        "  float lowerValue = .3;\n"
        "  float upperValue = .6;\n"
        "  for ( i=0; i<3; i+=1)\n"
        "  {\n"
        "    noisyColor[i] = (noisyColor[i] + 1.0) / 2.0;\n"
        "    if (noisyColor[i] < lowerValue) \n"
        "    {\n"
        "      noisyColor[i] = lowerValue;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "      if (noisyColor[i] < upperValue)\n"
        "      {\n"
        "        noisyColor[i] = upperValue;\n"
        "      }\n"
        "      else\n"
        "      {\n"
        "        noisyColor[i] = 1.0;\n"
        "      }\n"
        "    }\n"
        "  }\n"
        "  fragOutput0.rgb = opacity * vec3(ambientColor + noisyColor * "
        "diffuse + specular);\n"
        "  fragOutput0.a = opacity;\n",
        false // only do it once
    );
#endif
    vtkNew<vtkShaderCallback> myCallback;
    myCallback->Renderer = renderer;
    myCallback->k = 1;
    mapper->AddObserver(vtkCommand::UpdateShaderEvent, myCallback);
  }

  // Setup viewports for the renderers
  int rendererSize = 250;
  unsigned int xGridDimensions = 3;
  unsigned int yGridDimensions = 3;
  renderWindow->SetSize(rendererSize * xGridDimensions,
                        rendererSize * yGridDimensions);
  for (int row = 0; row < static_cast<int>(yGridDimensions); row++)
  {
    for (int col = 0; col < static_cast<int>(xGridDimensions); col++)
    {
      int index = row * xGridDimensions + col;
      // (xmin, ymin, xmax, ymax)
      double viewport[4] = {static_cast<double>(col) * rendererSize /
                                (xGridDimensions * rendererSize),
                            static_cast<double>(yGridDimensions - (row + 1)) *
                                rendererSize / (yGridDimensions * rendererSize),
                            static_cast<double>(col + 1) * rendererSize /
                                (xGridDimensions * rendererSize),
                            static_cast<double>(yGridDimensions - row) *
                                rendererSize /
                                (yGridDimensions * rendererSize)};
      renderers[index]->SetViewport(viewport);
    }
  }

  vtkNew<vtkRenderWindowInteractor> interactor;
  interactor->SetRenderWindow(renderWindow);

  renderWindow->Render();
  renderWindow->SetWindowName("BozoShaderDemo");
  renderWindow->Render();
  interactor->Start();

  return EXIT_SUCCESS;
}

namespace {
vtkSmartPointer<vtkPolyData> ReadPolyData(const char* fileName)
{
  vtkSmartPointer<vtkPolyData> polyData;
  std::string extension =
      vtksys::SystemTools::GetFilenameExtension(std::string(fileName));
  if (extension == ".ply")
  {
    vtkNew<vtkPLYReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".vtp")
  {
    vtkNew<vtkXMLPolyDataReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".obj")
  {
    vtkNew<vtkOBJReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".stl")
  {
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".vtk")
  {
    vtkNew<vtkPolyDataReader> reader;
    reader->SetFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else if (extension == ".g")
  {
    vtkNew<vtkBYUReader> reader;
    reader->SetGeometryFileName(fileName);
    reader->Update();
    polyData = reader->GetOutput();
  }
  else
  {
    vtkNew<vtkSphereSource> source;
    source->SetPhiResolution(25);
    source->SetThetaResolution(25);
    source->Update();
    polyData = source->GetOutput();
  }
  return polyData;
}
} // namespace
