#include <vtkActor.h>
#include <vtkAnnotatedCubeActor.h>
#include <vtkAxesActor.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkCameraOrientationWidget.h>
#include <vtkCaptionActor2D.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLookupTable.h>
#include <vtkMatrix4x4.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkPolyDataReader.h>
#include <vtkProp3D.h>
#include <vtkPropAssembly.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSliderRepresentation2D.h>
#include <vtkSliderWidget.h>
#include <vtkSmartPointer.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

#include <vtk_cli11.h>
#include <vtk_jsoncpp.h>

#include <array>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

namespace {
struct Parameters
{
  std::vector<std::string> names;
  std::map<std::string, std::string> colors;
  std::map<std::string, int> indices;
  std::map<std::string, std::string> orientation;
  std::map<std::string, double> opacity;
  std::map<std::string, std::string> vtkFiles;
  std::vector<std::string> fig_129b;
  std::vector<std::string> fig_129cd;

  bool parsedOk{false};
};

/**
 * Take a string and convert it to lowercase.
 *
 * See: https://en.cppreference.com/w/cpp/string/byte/tolower
 * Only works with ASCII characters.
 *
 * @param s: The string to be converted to lowercase.
 *
 * @return The lowercase version of the string.
 */
std::string ToLowerCase(std::string s);

/**
 * Read the parameters from a json file and check that the file paths exist.
 *
 * @param fnPath: The path to the json file.
 * @param parameters:  The parameters.
 */
void ParseJSON(const fs::path fnPath, Parameters& parameters);

/**
 * Create the lookup table for the frog tissues.
 *
 * Each table value corresponds the color of one of the frog tissues.
 *
 * @param indices: The tissue name and index.
 * @param colors: The tissue name and color.
 * @return: The lookup table.
 */
vtkNew<vtkLookupTable>
CreateTissueLUT(std::map<std::string, int> const& indices,
                std::map<std::string, std::string>& colors);

class SliceOrder
{
  // clang-format off
  /*
    These transformations permute image and other geometric data to maintain
      proper orientation regardless of the acquisition order. After applying
      these transforms with vtkTransformFilter, a view up of 0, -1, 0 will
      result in the body part facing the viewer.
     
    NOTE: Some transformations have a -1 scale factor for one of the components.
          To ensure proper polygon orientation and normal direction,
            you must apply the vtkPolyDataNormals filter.

    Naming (the nomenclature is medical):
      si - superior to inferior (top to bottom)
      is - inferior to superior (bottom to top)
      ap - anterior to posterior (front to back)
      pa - posterior to anterior (back to front)
      lr - left to right
      rl - right to left

    */
  // clang-format on

public:
  /**
   * Generate the transforms corresponding to the slice order.
   */
  SliceOrder();

  virtual ~SliceOrder() = default;

public:
  /**
   * Returns the vtkTransform corresponding to the slice order.
   *
   * @param sliceOrder: The slice order.
   * @return The vtkTransform corresponding to the slice order.
   */
  vtkSmartPointer<vtkTransform> Get(std::string const& sliceOrder);

  /**
   * Print the homogenous matrix corresponding to the slice order.
   *
   * @param order: The slice order.
   */
  void PrintTransform(std::string const& order);

  /**
   * Print all the homogenous matrices corresponding to the slice orders.
   *
   */
  void PrintAlltransforms();

private:
  std::map<std::string, vtkSmartPointer<vtkTransform>> transform;
};

/**
 * @param scale: Sets the scale and direction of the axes.
 * @param xyzLabels: Labels for the axes.
 * @return The axes actor.
 */
vtkNew<vtkAxesActor> MakeAxesActor(std::array<double, 3>& scale,
                                   std::array<std::string, 3> const& xyzLabels);

/**
 * @param cubeLabels: The labels for the cube faces.
 * @param colors: Used to set the colors of the cube faces.
 * @return The annotated cube actor.
 */
vtkNew<vtkAnnotatedCubeActor>
MakeAnnotatedCubeActor(std::array<std::string, 6> const& cubeLabels,
                       vtkNamedColors* colors);

/**
 * @param labelSelector: The selector used to define labels for the axes and
 * cube.
 * @param colors: Used to set the colors of the cube faces.
 * @return The combined axes and annotated cube prop.
 */
vtkNew<vtkPropAssembly> MakeCubeActor(std::string const& labelSelector,
                                      vtkNamedColors* colors);

class SliderCallbackOpacity : public vtkCallbackCommand
{
public:
  static SliderCallbackOpacity* New()
  {
    return new SliderCallbackOpacity;
  }
  virtual void Execute(vtkObject* caller, unsigned long, void*)
  {
    vtkSliderWidget* sliderWidget = reinterpret_cast<vtkSliderWidget*>(caller);
    double value = static_cast<vtkSliderRepresentation2D*>(
                       sliderWidget->GetRepresentation())
                       ->GetValue();
    this->property->SetOpacity(value);
  }
  SliderCallbackOpacity() : property(nullptr)
  {
  }
  vtkProperty* property;
};
struct SliderProperties
{
  double tubeWidth{0.004};
  double sliderLength{0.015};
  double sliderWidth{0.008};
  double endCapLength{0.008};
  double endCapWidth{0.02};
  double titleHeight{0.02};
  double labelHeight{0.02};

  double valueMinimum{0.0};
  double valueMaximum{1.0};
  double valueInitial{1.0};

  std::array<double, 2> p1{0.02, 0.1};
  std::array<double, 2> p2{0.18, 0.1};

  std::string title;

  std::string titleColor{"Black"};
  std::string labelColor{"Black"};
  std::string valueColor{"DarkSlateGray"};
  std::string sliderColor{"BurlyWood"};
  std::string selectedColor{"Lime"};
  std::string barColor{"Black"};
  std::string barEndsColor{"Indigo"};
};

/**
 * @param properties: The slider properties.
 * @param lut: The color lookup table.
 * @param idx: The tissue index.
 * @return The slider widget.
 */
vtkNew<vtkSliderWidget> MakeSliderWidget(SliderProperties const& sp,
                                         vtkLookupTable* lut, int const& idx);

class SliderToggleCallback : public vtkCallbackCommand
{
public:
  SliderToggleCallback() = default;

  static SliderToggleCallback* New()
  {
    return new SliderToggleCallback;
  }

  /*
   * Create a vtkCallbackCommand and reimplement it.
   *
   */
  void Execute(vtkObject* caller, unsigned long /*evId*/, void*) override
  {
    // Note the use of reinterpret_cast to cast the caller to the expected type.
    auto rwi = reinterpret_cast<vtkRenderWindowInteractor*>(caller);

    // Get the keypress
    std::string key = rwi->GetKeySym();
    if (key == "n")
    {
      for (auto const& p : *this->sliders)
      {
        if (p.second->GetEnabled())
        {
          p.second->Off();
        }
        else
        {
          p.second->On();
        }
      }
    }
  }

  /**
   * For handling the sliders.
   *
   * @param sliders The sliders.
   *
   */
  void SetParameters(
      std::map<std::string, vtkSmartPointer<vtkSliderWidget>>* sliders)
  {
    this->sliders = sliders;
  }

private:
  SliderToggleCallback(const SliderToggleCallback&) = delete;
  void operator=(const SliderToggleCallback&) = delete;

  std::map<std::string, vtkSmartPointer<vtkSliderWidget>>* sliders{nullptr};
};

} // namespace

int main(int argc, char* argv[])
{
  CLI::App app{"View surfaces of a segmented frog dataset using preprocessed "
               "VTK tissue files."};

  // Define options
  std::string fileName;
  app.add_option("fileName", fileName,
                 "The path to the JSON file e.g. Frog_vtk.json.")
      ->required()
      ->check(CLI::ExistingFile);

  std::vector<std::string> chosenTissues;
  app.add_option("-t", chosenTissues, "Select one or more tissues.");
  auto noSliders = false;
  app.add_flag("-n", noSliders, "No sliders");

  std::array<bool, 6> view{false, false, false, false};
  auto* ogroup = app.add_option_group(
      "view",
      "Select a view corresponding to Fig 12-9 in the VTK Textbook. Only one "
      "or none of these can be selected.");
  ogroup->add_flag("-a", view[0],
                   "The view corresponds to Fig 12-9a in the VTK Textbook");
  ogroup->add_flag("-b", view[1],
                   "The view corresponds to Fig 12-9b in the VTK Textbook");
  ogroup->add_flag("-c", view[2],
                   "The view corresponds to Fig 12-9c in the VTK Textbook");
  ogroup->add_flag("-d", view[3],
                   "The view corresponds to Fig 12-9d in the VTK Textbook");
  ogroup->add_flag(
      "-l", view[4],
      "The view corresponds to looking down on the anterior surface");
  ogroup->add_flag("-p", view[5],
                   "The view corresponds to looking down on the posterior "
                   "surface (the default)");

  CLI11_PARSE(app, argc, argv);

  auto selectCount = std::count_if(view.begin(), view.end(),
                                   [=](const bool& e) { return e == true; });
  if (selectCount > 1)
  {
    std::cerr << "Only one or none of the options -a, -b, -c, -d, -l, -p can "
                 "be selected;"
              << std::endl;
    return EXIT_FAILURE;
  }

  auto fnPath = fs::path(fileName);
  if (!fnPath.has_extension())
  {
    fnPath.replace_extension(".json");
  }
  if (!fs::is_regular_file(fnPath))
  {
    std::cerr << "Unable to find: " << fnPath << std::endl;
    return EXIT_FAILURE;
  }

  Parameters parameters;
  ParseJSON(fnPath, parameters);
  if (!parameters.parsedOk)
  {
    return EXIT_FAILURE;
  }

  auto tissues = parameters.names;
  auto indices = parameters.indices;

  auto lut = CreateTissueLUT(parameters.indices, parameters.colors);

  char selectFigure{'\0'};
  if (selectCount == 1)
  {
    if (view[0])
    {
      selectFigure = 'a';
    }
    else if (view[1])
    {
      // No skin.
      tissues = parameters.fig_129b;
      selectFigure = 'b';
    }
    else if (view[2])
    {
      // No skin, blood and skeleton.
      tissues = parameters.fig_129cd;
      selectFigure = 'c';
    }
    else if (view[3])
    {
      // No skin, blood and skeleton.
      tissues = parameters.fig_129cd;
      selectFigure = 'd';
    }
    else if (view[4])
    {
      // Looking down on the anterior surface.
      selectFigure = 'l';
    }
    else // The default
    {
      // Looking down on the posterior surface.
      selectFigure = 'p';
    }
  }

  if (!chosenTissues.empty())
  {
    for (auto i = 0; i < chosenTissues.size(); ++i)
    {
      chosenTissues[i] = ToLowerCase(chosenTissues[i]);
    }
    std::vector<std::string> res;
    auto has_brainbin{false};
    if (std::find(chosenTissues.begin(), chosenTissues.end(), "brainbin") !=
        chosenTissues.end())
    {
      std::cout << "Using brainbin instead of brain." << std::endl;
      res.push_back("brainbin");
      indices.erase("brain");
      indices["brainbin"] = 2;
      parameters.colors.erase("brain");
      parameters.colors["brainbin"] = "beige";
      has_brainbin = true;
    }
    for (auto const& ct : chosenTissues)
    {
      if (has_brainbin && (ct == "brain" || ct == "brainbin"))
      {
        continue;
      }
      if (std::find(tissues.begin(), tissues.end(), ct) != tissues.end())
      {
        res.push_back(ct);
      }
      else
      {
        std::cout << "Tissue: " << ct << " is not available." << std::endl;
        return EXIT_FAILURE;
      }
    }
    if (res.size() == 1 && res[0] == "skin")
    {
      parameters.opacity["skin"] = 1.0;
    }
    tissues = res;
  }

  vtkNew<vtkNamedColors> colors;
  colors->SetColor("ParaViewBkg",
                   std::array<unsigned char, 4>{82, 87, 110, 255}.data());

  // Setup render window, renderer, and interactor.
  vtkNew<vtkRenderer> ren;
  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(ren);
  vtkNew<vtkRenderWindowInteractor> iRen;
  iRen->SetRenderWindow(renWin);
  vtkNew<vtkInteractorStyleTrackballCamera> style;
  iRen->SetInteractorStyle(style);

  SliceOrder so;
  // so.PrintAlltransforms();

  size_t colorSize{0};
  for (auto const& p : parameters.colors)
  {
    colorSize = std::max(colorSize, p.second.size());
  }
  size_t nameSize{0};
  for (auto const& p : parameters.names)
  {
    nameSize = std::max(nameSize, p.size());
  }

  std::map<std::string, vtkSmartPointer<vtkSliderWidget>> sliders;
  auto leftStepSize = 1.0 / 9;
  auto leftPosY = 0.275;
  auto leftPosX0 = 0.02;
  auto leftPosX1 = 0.18;
  auto rightStepSize = 1.0 / 9;
  auto rightPosY = 0.05;
  auto rightPosX0 = 0.8 + 0.02;
  auto rightPosX1 = 0.8 + 0.18;

  auto sliderCount = 0;

  std::string line(7 + nameSize + colorSize, '-');

  std::cout << line << '\n'
            << std::setw(nameSize) << std::left << "Tissue"
            << " Label "
            << "Color" << '\n'
            << line << std::endl;
  auto intSize = 2;

  for (auto const& tissue : tissues)
  {
    vtkNew<vtkPolyDataReader> reader;
    reader->SetFileName(parameters.vtkFiles[tissue].c_str());
    reader->Update();

    auto trans = so.Get(parameters.orientation[tissue]);
    trans->Scale(1, -1, -1);

    vtkNew<vtkTransformPolyDataFilter> tf;
    tf->SetInputConnection(reader->GetOutputPort());
    tf->SetTransform(trans);
    tf->SetInputConnection(reader->GetOutputPort());

    vtkNew<vtkPolyDataNormals> normals;
    normals->SetInputConnection(tf->GetOutputPort());
    normals->SetFeatureAngle(60.0);

    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(normals->GetOutputPort());

    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetOpacity(parameters.opacity[tissue]);
    actor->GetProperty()->SetDiffuseColor(lut->GetTableValue(indices[tissue]));
    actor->GetProperty()->SetSpecular(0.2);
    actor->GetProperty()->SetSpecularPower(10);

    ren->AddActor(actor);

    if (!noSliders)
    {
      auto sp = SliderProperties();

      sp.valueInitial = parameters.opacity[tissue];
      sp.title = tissue;

      // Screen coordinates.
      if (sliderCount < 7)
      {
        sp.p1[0] = leftPosX0;
        sp.p1[1] = leftPosY;
        sp.p2[0] = leftPosX1;
        sp.p2[1] = leftPosY;
        leftPosY += leftStepSize;
      }
      else
      {
        sp.p1[0] = rightPosX0;
        sp.p1[1] = rightPosY;
        sp.p2[0] = rightPosX1;
        sp.p2[1] = rightPosY;
        rightPosY += rightStepSize;
      }

      auto sliderWidget = MakeSliderWidget(sp, lut, parameters.indices[tissue]);

      sliderWidget->SetInteractor(iRen);
      sliderWidget->SetAnimationModeToAnimate();
      sliderWidget->EnabledOn();

      vtkNew<SliderCallbackOpacity> cb;
      cb->property = actor->GetProperty();
      sliderWidget->AddObserver(vtkCommand::InteractionEvent, cb);
      sliders[tissue] = sliderWidget;
      sliderCount += 1;
    }
    std::cout << std::setw(nameSize) << std::left << tissue << " "
              << std::setw(intSize + 3) << std::right << indices[tissue] << " "
              << std::setw(colorSize) << std::left << parameters.colors[tissue]
              << std::endl;
  }
  std::cout << line << std::endl;

  if (noSliders)
  {
    renWin->SetSize(1024, 1024);
  }
  else
  {
    renWin->SetSize(1024 + 400, 1024);
  }
  renWin->SetWindowName("FroggieView");

  ren->SetBackground(colors->GetColor3d("ParaViewBkg").GetData());

  auto camera = ren->GetActiveCamera();
  // Superior Anterior Left
  auto labels{"sal"};
  if (selectFigure == 'a')
  {
    // Fig 12-9a in the VTK Textbook
    camera->SetPosition(495.722368, -447.474954, -646.308030);
    camera->SetFocalPoint(137.612066, -40.962376, -195.171023);
    camera->SetViewUp(-0.323882, -0.816232, 0.478398);
    camera->SetDistance(704.996499);
    camera->SetClippingRange(319.797039, 1809.449285);
  }
  else if (selectFigure == 'b')
  {
    // Fig 12-9b in the VTK Textbook
    camera->SetPosition(478.683494, -420.477744, -643.112038);
    camera->SetFocalPoint(135.624874, -36.478435, -210.614440);
    camera->SetViewUp(-0.320495, -0.820148, 0.473962);
    camera->SetDistance(672.457328);
    camera->SetClippingRange(307.326771, 1765.990822);
  }
  else if (selectFigure == 'c')
  {
    // Fig 12-9c in the VTK Textbook
    camera->SetPosition(201.363313, -147.260834, -229.885066);
    camera->SetFocalPoint(140.626206, -75.857216, -162.352531);
    camera->SetViewUp(-0.425438, -0.786048, 0.448477);
    camera->SetDistance(115.534047);
    camera->SetClippingRange(7.109870, 854.091718);
  }
  else if (selectFigure == 'd')
  {
    // Fig 12-9d in the VTK Textbook
    camera->SetPosition(115.361727, -484.656410, -6.193827);
    camera->SetFocalPoint(49.126343, 98.501094, 1.323317);
    camera->SetViewUp(-0.649127, -0.083475, 0.756086);
    camera->SetDistance(586.955116);
    camera->SetClippingRange(360.549218, 866.876230);
  }
  else if (selectFigure == 'l')
  {
    // Orient so that we look down on the anterior surface and
    //  the superior surface faces the top of the screen.
    // Left Superior Anterior
    labels = "lsa";
    vtkNew<vtkTransform> transform;
    transform->SetMatrix(camera->GetModelTransformMatrix());
    transform->RotateY(90);
    transform->RotateZ(90);
    camera->SetModelTransformMatrix(transform->GetMatrix());
    ren->ResetCamera();
  }
  else
  {
    // Orient so that we look down on the posterior surface and
    //  the superior surface faces the top of the screen.
    // Right Superior Posterior
    labels = "rsp";
    vtkNew<vtkTransform> transform;
    transform->SetMatrix(camera->GetModelTransformMatrix());
    transform->RotateY(-90);
    transform->RotateZ(90);
    camera->SetModelTransformMatrix(transform->GetMatrix());
    ren->ResetCamera();
  }

  vtkNew<vtkCameraOrientationWidget> cow;
  cow->SetParentRenderer(ren);
  if (noSliders)
  {
    // Turn off if you do not want it.
    cow->On();
    cow->EnabledOn();
  }
  else
  {
    cow->Off();
    cow->EnabledOff();
  }

  auto axes = MakeCubeActor(labels, colors);
  vtkNew<vtkOrientationMarkerWidget> om;
  om->SetOrientationMarker(axes);
  // Position upper left in the viewport.
  // om->SetViewport(0.0, 0.8, 0.2, 1.0);
  // Position lower left in the viewport.
  om->SetViewport(0, 0, 0.2, 0.2);
  om->SetInteractor(iRen);
  om->EnabledOn();
  om->InteractiveOn();

  renWin->Render();

  vtkNew<SliderToggleCallback> sliderToggle;
  sliderToggle->SetParameters(&sliders);
  iRen->AddObserver(vtkCommand::KeyPressEvent, sliderToggle);

  iRen->Start();

  return EXIT_SUCCESS;
}

namespace {

std::string ToLowerCase(std::string s)
{
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); } // correct
  );
  return s;
}

void ParseJSON(const fs::path fnPath, Parameters& parameters)
{
  std::ifstream ifs(fnPath);
  Json::Value root;

  if (ifs)
  {
    std::string str;
    std::string errors;
    Json::CharReaderBuilder builder{};
    auto reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());

    std::ostringstream ss;
    ss << ifs.rdbuf(); // Read in the file comtents
    str = ss.str();
    auto parsingSuccessful =
        reader->parse(str.c_str(), str.c_str() + str.size(), &root, &errors);
    ifs.close();
    if (!parsingSuccessful)
    {
      std::cout << errors << std::endl;
      parameters.parsedOk = false;
      return;
    }
    parameters.parsedOk = true;
  }
  else
  {
    std::cerr << "Unable to open: " << fnPath << std::endl;
    parameters.parsedOk = false;
  }

  // Get the parameters that we need.
  fs::path vtkPath;
  std::vector<std::string> fileNames;
  for (Json::Value::const_iterator outer = root.begin(); outer != root.end();
       ++outer)
  {
    if (outer.name() == "files")
    {
      std::string path;
      for (Json::Value::const_iterator pth = root["files"].begin();
           pth != root["files"].end(); ++pth)
      {
        if (pth.name() == "root")
        {
          vtkPath = fs::path(pth->asString());
        }
        if (pth.name() == "vtk_files")
        {
          for (Json::Value::const_iterator fls =
                   root["files"]["vtk_files"].begin();
               fls != root["files"]["vtk_files"].end(); ++fls)
          {
            fileNames.push_back(fls->asString());
          }
        }
      }
    }
    if (outer.name() == "tissues")
    {
      for (Json::Value::const_iterator tc = root["tissues"]["names"].begin();
           tc != root["tissues"]["names"].end(); ++tc)
      {
        parameters.names.push_back(tc->asString());
      }
      for (Json::Value::const_iterator tc = root["tissues"]["indices"].begin();
           tc != root["tissues"]["indices"].end(); ++tc)
      {
        parameters.indices[tc.name()] = tc->asInt();
      }
      for (Json::Value::const_iterator tc = root["tissues"]["colors"].begin();
           tc != root["tissues"]["colors"].end(); ++tc)
      {
        parameters.colors[tc.name()] = tc->asString();
      }
      for (Json::Value::const_iterator tc = root["tissues"]["indices"].begin();
           tc != root["tissues"]["indices"].end(); ++tc)
      {
        parameters.indices[tc.name()] = tc->asInt();
      }
      for (Json::Value::const_iterator tc =
               root["tissues"]["orientation"].begin();
           tc != root["tissues"]["orientation"].end(); ++tc)
      {
        parameters.orientation[tc.name()] = tc->asString();
      }
      for (Json::Value::const_iterator tc = root["tissues"]["opacity"].begin();
           tc != root["tissues"]["opacity"].end(); ++tc)
      {
        parameters.opacity[tc.name()] = tc->asDouble();
      }
    }
    if (outer.name() == "figures")
    {
      for (Json::Value::const_iterator c = root["figures"]["fig12-9b"].begin();
           c != root["figures"]["fig12-9b"].end(); ++c)
      {
        parameters.fig_129b.push_back(c->asString());
      }
      for (Json::Value::const_iterator c = root["figures"]["fig12-9cd"].begin();
           c != root["figures"]["fig12-9cd"].end(); ++c)
      {
        parameters.fig_129cd.push_back(c->asString());
      }
    }
  }

  //  Build and check the paths.
  if (!fileNames.empty())
  {
    if (fileNames.size() != 17)
    {
      std::cerr << "Expected seventeen file names.";
      parameters.parsedOk = false;
    }
    else
    {
      for (size_t i = 0; i < fileNames.size(); i++)
      {
        auto pth = fnPath.parent_path() / vtkPath / fs::path(fileNames[i]);
        fileNames[i] = pth.make_preferred().string();
        if (!(fs::is_regular_file(pth) && fs::exists(pth)))
        {
          std::cerr << "Not a file or path does not exist: " << fileNames[i]
                    << std::endl;
          parameters.parsedOk = false;
        }
        else
        {
          parameters.vtkFiles[pth.stem().string()] =
              pth.make_preferred().string();
        }
      }
    }
  }
  else
  {
    std::cerr << "Expected .vtk file names in the JSON file.";
    parameters.parsedOk = false;
  }
}

vtkNew<vtkLookupTable>
CreateTissueLUT(std::map<std::string, int> const& indices,
                std::map<std::string, std::string>& colors)
{
  vtkNew<vtkLookupTable> lut;
  lut->SetNumberOfColors(colors.size());
  lut->SetTableRange(0, colors.size() - 1);
  lut->Build();

  vtkNew<vtkNamedColors> nc;

  for (auto const& p : indices)
  {
    lut->SetTableValue(p.second, nc->GetColor4d(colors[p.first]).GetData());
  }

  return lut;
}

SliceOrder::SliceOrder()
{
  vtkNew<vtkMatrix4x4> si_mat;
  si_mat->Zero();
  si_mat->SetElement(0, 0, 1);
  si_mat->SetElement(1, 2, 1);
  si_mat->SetElement(2, 1, -1);
  si_mat->SetElement(3, 3, 1);

  vtkNew<vtkMatrix4x4> is_mat;
  is_mat->Zero();
  is_mat->SetElement(0, 0, 1);
  is_mat->SetElement(1, 2, -1);
  is_mat->SetElement(2, 1, -1);
  is_mat->SetElement(3, 3, 1);

  vtkNew<vtkMatrix4x4> lr_mat;
  lr_mat->Zero();
  lr_mat->SetElement(0, 2, -1);
  lr_mat->SetElement(1, 1, -1);
  lr_mat->SetElement(2, 0, 1);
  lr_mat->SetElement(3, 3, 1);

  vtkNew<vtkMatrix4x4> rl_mat;
  rl_mat->Zero();
  rl_mat->SetElement(0, 2, 1);
  rl_mat->SetElement(1, 1, -1);
  rl_mat->SetElement(2, 0, 1);
  rl_mat->SetElement(3, 3, 1);

  // The previous transforms assume radiological views of the slices
  //  (viewed from the feet).
  // Other modalities such as physical sectioning may view from the head.
  // The following transforms modify the original with a 180° rotation about y

  vtkNew<vtkMatrix4x4> hf_mat;
  hf_mat->Zero();
  hf_mat->SetElement(0, 0, -1);
  hf_mat->SetElement(1, 1, 1);
  hf_mat->SetElement(2, 2, -1);
  hf_mat->SetElement(3, 3, 1);

  vtkNew<vtkTransform> si_trans;
  si_trans->SetMatrix(si_mat);
  this->transform["si"] = si_trans;

  vtkNew<vtkTransform> is_trans;
  is_trans->SetMatrix(is_mat);
  this->transform["is"] = is_trans;

  vtkNew<vtkTransform> ap_trans;
  ap_trans->Scale(1, -1, 1);
  this->transform["ap"] = ap_trans;

  vtkNew<vtkTransform> pa_trans;
  pa_trans->Scale(1, -1, -1);
  this->transform["pa"] = pa_trans;

  vtkNew<vtkTransform> lr_trans;
  lr_trans->SetMatrix(lr_mat);
  this->transform["lr"] = lr_trans;

  vtkNew<vtkTransform> rl_trans;
  lr_trans->SetMatrix(rl_mat);
  this->transform["rl"] = rl_trans;

  vtkNew<vtkTransform> hf_trans;
  hf_trans->SetMatrix(hf_mat);
  this->transform["hf"] = hf_trans;

  vtkNew<vtkTransform> hf_si_trans;
  hf_si_trans->SetMatrix(hf_mat);
  hf_si_trans->Concatenate(si_mat);
  this->transform["hfsi"] = hf_si_trans;

  vtkNew<vtkTransform> hf_is_trans;
  hf_is_trans->SetMatrix(hf_mat);
  hf_is_trans->Concatenate(is_mat);
  this->transform["hfis"] = hf_is_trans;

  vtkNew<vtkTransform> hf_ap_trans;
  hf_ap_trans->SetMatrix(hf_mat);
  hf_ap_trans->Scale(1, -1, 1);
  this->transform["hfap"] = hf_ap_trans;

  vtkNew<vtkTransform> hf_pa_trans;
  hf_pa_trans->SetMatrix(hf_mat);
  hf_pa_trans->Scale(1, -1, -1);
  this->transform["hfpa"] = hf_pa_trans;

  vtkNew<vtkTransform> hf_lr_trans;
  hf_lr_trans->SetMatrix(hf_mat);
  hf_lr_trans->Concatenate(lr_mat);
  this->transform["hflr"] = hf_lr_trans;

  vtkNew<vtkTransform> hf_rl_trans;
  hf_rl_trans->SetMatrix(hf_mat);
  hf_rl_trans->Concatenate(rl_mat);
  this->transform["hfrl"] = hf_rl_trans;

  // Identity
  this->transform["I"] = vtkNew<vtkTransform>();

  // Zero
  vtkNew<vtkTransform> z_trans;
  z_trans->Scale(0, 0, 0);
  this->transform["Z"] = z_trans;
}

void SliceOrder::PrintTransform(std::string const& order)
{
  auto m = this->transform[order]->GetMatrix();
  std::ostringstream os;
  os.setf(std::ios_base::fmtflags(), std::ios_base::floatfield);
  os << order << '\n';
  for (int i = 0; i < 4; ++i)
  {
    for (int j = 0; j < 4; ++j)
    {
      if (j < 3)
      {
        os << std::setw(6) << std::right << std::setprecision(2)
           << m->GetElement(i, j) << " ";
      }
      else
      {
        os << std::setw(6) << std::right << m->GetElement(i, j) << '\n';
      }
    }
  }
  std::cout << os.str() << '\n';
  os.str("");
}

void SliceOrder::PrintAlltransforms()
{
  for (auto const& p : this->transform)
  {
    PrintTransform(p.first);
  }
}

vtkSmartPointer<vtkTransform> SliceOrder::Get(std::string const& sliceOrder)
{
  return this->transform[sliceOrder];
}

vtkNew<vtkAxesActor> MakeAxesActor(std::array<double, 3>& scale,
                                   std::array<std::string, 3> const& xyzLabels)
{
  vtkNew<vtkAxesActor> axes;
  axes->SetScale(scale.data());
  axes->SetShaftTypeToCylinder();
  axes->SetXAxisLabelText(xyzLabels[0].c_str());
  axes->SetYAxisLabelText(xyzLabels[1].c_str());
  axes->SetZAxisLabelText(xyzLabels[2].c_str());
  axes->SetCylinderRadius(0.5 * axes->GetCylinderRadius());
  axes->SetConeRadius(1.025 * axes->GetConeRadius());
  axes->SetSphereRadius(1.5 * axes->GetSphereRadius());
  auto tprop = axes->GetXAxisCaptionActor2D()->GetCaptionTextProperty();
  tprop->ItalicOn();
  tprop->ShadowOn();
  tprop->SetFontFamilyToTimes();
  // Use the same text properties on the other two axes.
  axes->GetYAxisCaptionActor2D()->GetCaptionTextProperty()->ShallowCopy(tprop);
  axes->GetZAxisCaptionActor2D()->GetCaptionTextProperty()->ShallowCopy(tprop);
  return axes;
}

vtkNew<vtkAnnotatedCubeActor>
MakeAnnotatedCubeActor(std::array<std::string, 6> const& cubeLabels,
                       vtkNamedColors* colors)
{
  // A cube with labeled faces.
  vtkNew<vtkAnnotatedCubeActor> cube;
  cube->SetXPlusFaceText(cubeLabels[0].c_str());
  cube->SetXMinusFaceText(cubeLabels[1].c_str());
  cube->SetYPlusFaceText(cubeLabels[2].c_str());
  cube->SetYMinusFaceText(cubeLabels[3].c_str());
  cube->SetZPlusFaceText(cubeLabels[4].c_str());
  cube->SetZMinusFaceText(cubeLabels[5].c_str());
  cube->SetFaceTextScale(0.5);
  cube->GetCubeProperty()->SetColor(colors->GetColor3d("Gainsboro").GetData());

  cube->GetTextEdgesProperty()->SetColor(
      colors->GetColor3d("LightSlateGray").GetData());

  // Change the vector text colors.
  cube->GetXPlusFaceProperty()->SetColor(
      colors->GetColor3d("Tomato").GetData());
  cube->GetXMinusFaceProperty()->SetColor(
      colors->GetColor3d("Tomato").GetData());
  cube->GetYPlusFaceProperty()->SetColor(
      colors->GetColor3d("DeepSkyBlue").GetData());
  cube->GetYMinusFaceProperty()->SetColor(
      colors->GetColor3d("DeepSkyBlue").GetData());
  cube->GetZPlusFaceProperty()->SetColor(
      colors->GetColor3d("SeaGreen").GetData());
  cube->GetZMinusFaceProperty()->SetColor(
      colors->GetColor3d("SeaGreen").GetData());
  return cube;
}

vtkNew<vtkPropAssembly> MakeCubeActor(std::string const& labelSelector,
                                      vtkNamedColors* colors)
{
  std::array<std::string, 3> xyzLabels;
  std::array<std::string, 6> cubeLabels;
  std::array<double, 3> scale;
  if (labelSelector == "sal")
  {
    // xyzLabels = std::array<std::string,3>{"S", "A", "L"};
    xyzLabels = std::array<std::string, 3>{"+X", "+Y", "+Z"};
    cubeLabels = std::array<std::string, 6>{"S", "I", "A", "P", "L", "R"};
    scale = std::array<double, 3>{1.5, 1.5, 1.5};
  }
  else if (labelSelector == "rsp")
  {
    // xyzLabels = std::array<std::string, 3>{"R", "S", "P"};
    xyzLabels = std::array<std::string, 3>{"+X", "+Y", "+Z"};
    cubeLabels = std::array<std::string, 6>{"R", "L", "S", "I", "P", "A"};
    scale = std::array<double, 3>{1.5, 1.5, 1.5};
  }
  else if (labelSelector == "lsa")
  {
    // xyzLabels = std::array<std::string, 3>{"L", "S", "A"};
    xyzLabels = std::array<std::string, 3>{"+X", "+Y", "+Z"};
    cubeLabels = std::array<std::string, 6>{"L", "R", "S", "I", "A", "P"};
    scale = std::array<double, 3>{1.5, 1.5, 1.5};
  }
  else
  {
    xyzLabels = std::array<std::string, 3>{"+X", "+Y", "+Z"};
    cubeLabels = std::array<std::string, 6>{"+X", "-X", "+Y", "-Y", "+Z", "-Z"};
    scale = std::array<double, 3>{1.5, 1.5, 1.5};
  }

  // We are combining a vtkAxesActor and a vtkAnnotatedCubeActor
  // into a vtkPropAssembly
  auto cube = MakeAnnotatedCubeActor(cubeLabels, colors);
  auto axes = MakeAxesActor(scale, xyzLabels);

  // Combine orientation markers into one with an assembly.
  vtkNew<vtkPropAssembly> assembly;
  assembly->AddPart(axes);
  assembly->AddPart(cube);
  return assembly;
}

vtkNew<vtkSliderWidget> MakeSliderWidget(SliderProperties const& sp,
                                         vtkLookupTable* lut, int const& idx)
{
  vtkNew<vtkSliderRepresentation2D> slider;

  slider->SetMinimumValue(sp.valueMinimum);
  slider->SetMaximumValue(sp.valueMaximum);
  slider->SetValue(sp.valueInitial);
  slider->SetTitleText(sp.title.c_str());

  slider->GetPoint1Coordinate()->SetCoordinateSystemToNormalizedDisplay();
  slider->GetPoint1Coordinate()->SetValue(sp.p1[0], sp.p1[1]);
  slider->GetPoint2Coordinate()->SetCoordinateSystemToNormalizedDisplay();
  slider->GetPoint2Coordinate()->SetValue(sp.p2[0], sp.p2[1]);

  slider->SetTubeWidth(sp.tubeWidth);
  slider->SetSliderLength(sp.sliderLength);
  slider->SetSliderWidth(sp.sliderWidth);
  slider->SetEndCapLength(sp.endCapLength);
  slider->SetEndCapWidth(sp.endCapWidth);
  slider->SetTitleHeight(sp.titleHeight);
  slider->SetLabelHeight(sp.labelHeight);

  vtkNew<vtkNamedColors> colors;
  // Set the colors of the slider components.
  // Change the color of the bar.
  slider->GetTubeProperty()->SetColor(
      colors->GetColor3d(sp.barColor).GetData());
  // Change the color of the ends of the bar.
  slider->GetCapProperty()->SetColor(
      colors->GetColor3d(sp.barEndsColor).GetData());
  // Change the color of the knob that slides.
  slider->GetSliderProperty()->SetColor(
      colors->GetColor3d(sp.sliderColor).GetData());
  // Change the color of the knob when the mouse is held on it.
  slider->GetSelectedProperty()->SetColor(
      colors->GetColor3d(sp.selectedColor).GetData());
  // Change the color of the text displaying the value.
  slider->GetLabelProperty()->SetColor(
      colors->GetColor3d(sp.valueColor).GetData());
  //  Use the one color for the labels.
  // slider->GetTitleProperty()->SetColor(colors->GetColor3d(sp.labelColor));
  // Change the color of the text indicating what the slider controls.
  if (idx >= 0 && idx < 16)
  {
    slider->GetTitleProperty()->SetColor(lut->GetTableValue(idx));
    slider->GetTitleProperty()->ShadowOff();
  }
  else
  {
    slider->GetTitleProperty()->SetColor(
        colors->GetColor3d(sp.titleColor).GetData());
  }

  vtkNew<vtkSliderWidget> sliderWidget;
  sliderWidget->SetRepresentation(slider);

  return sliderWidget;
}

} // namespace
