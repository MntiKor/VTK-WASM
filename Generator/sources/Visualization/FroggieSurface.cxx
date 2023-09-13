#include <vtkActor.h>
#include <vtkAnnotatedCubeActor.h>
#include <vtkAxesActor.h>
#include <vtkCamera.h>
#include <vtkCameraOrientationWidget.h>
#include <vtkCaptionActor2D.h>
#include <vtkDecimatePro.h>
#include <vtkFlyingEdges3D.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkImageIslandRemoval2D.h>
#include <vtkImageShrink3D.h>
#include <vtkImageThreshold.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLookupTable.h>
#include <vtkMarchingCubes.h>
#include <vtkMatrix4x4.h>
#include <vtkMetaImageReader.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProp3D.h>
#include <vtkPropAssembly.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkStripper.h>
#include <vtkTextProperty.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkWindowedSincPolyDataFilter.h>

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
#include <variant>

namespace fs = std::filesystem;

namespace {
struct Parameters
{
  std::vector<std::string> names;
  std::map<std::string, std::string> colors;
  std::map<std::string, std::string> mhdFiles;
  std::vector<std::string> fig_129b;
  std::vector<std::string> fig_129cd;
  std::map<std::string,
           std::map<std::string, std::variant<int, double, std::string>>>
      tissues;

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
 * Create the actor for a specific tissue.
 *
 * @param name: The tissue name.
 * @param tissue: The tissue parameters.
 * @param files: The path to the tissue files.
 * @param flying_edges: If true use flying edges.
 * @param decimate: If true decimate.
 * @param lut: The color lookup table for the tissues.
 * @param so: The transforms corresponding to the slice order.
 * @param actor: The actor.
 */
void CreateTissueActor(
    std::string const& name,
    std::map<std::string, std::variant<int, double, std::string>>& tissue,
    std::map<std::string, std::string>& files, bool const& flying_edges,
    bool const& decimate, vtkLookupTable& color_lut, SliceOrder& so,
    vtkActor* actor);

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
} // namespace

int main(int argc, char* argv[])
{
  CLI::App app{"Construct surfaces from a segmented frog dataset."};

  // Define options
  std::string fileName;
  app.add_option("fileName", fileName,
                 "The path to the JSON file e.g. Frog_mhd.json.")
      ->required()
      ->check(CLI::ExistingFile);

  bool flyingEdges = true;
  app.add_flag("-m{false},!-n", flyingEdges,
               "Use flying edges by default, marching cubes if set.");
  bool decimation{false};
  // -o: obliterate a synonym for decimation.
  app.add_flag("-o", decimation, "Decimate if set.");

  std::vector<std::string> chosenTissues;
  app.add_option("-t", chosenTissues, "Select one or more tissues.");

  std::array<bool, 6> view{false, false, false, false, false, false};
  auto* ogroup =
      app.add_option_group("view",
                           "Select the orientation of the frog. Only none or "
                           "one of these can be selected.");
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

  std::vector<std::string> tissues;
  std::map<std::string, int> indices;

  for (auto const& n : parameters.names)
  {
    if (n != "brainbin")
    {
      tissues.push_back(n);
      indices[n] = *std::get_if<int>(&parameters.tissues[n]["tissue"]);
    }
  }
  auto lut = CreateTissueLUT(indices, parameters.colors);

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
      parameters.tissues["skin"]["opacity"] = 1.0;
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

  std::string line(7 + nameSize + colorSize, '-');

  std::cout << line << '\n'
            << std::setw(nameSize) << std::left << "Tissue"
            << " Label "
            << "Color" << '\n'
            << line << std::endl;
  auto intSize = 2;

  for (auto const& name : tissues)
  {
    vtkNew<vtkActor> actor;
    CreateTissueActor(name, parameters.tissues[name], parameters.mhdFiles,
                      flyingEdges, decimation, *lut, so, actor);
    ren->AddActor(actor);
    std::cout << std::setw(nameSize) << std::left << name << " "
              << std::setw(intSize + 3) << std::right << indices[name] << " "
              << std::setw(colorSize) << std::left << parameters.colors[name]
              << std::endl;
  }
  std::cout << line << std::endl;

  renWin->SetSize(1024, 1024);
  renWin->SetWindowName("FroggieSurface");

  ren->SetBackground(colors->GetColor3d("ParaViewBkg").GetData());

  auto camera = ren->GetActiveCamera();
  // Superior Anterior Left
  auto labels{"sal"};
  if (selectFigure == 'a')
  {
    // Fig 12-9a in the VTK Textbook
    camera->SetPosition(742.731237, -441.329635, -877.192015);
    camera->SetFocalPoint(247.637687, 120.680880, -253.487473);
    camera->SetViewUp(-0.323882, -0.816232, 0.478398);
    camera->SetDistance(974.669585);
    camera->SetClippingRange(311.646383, 1803.630763);
  }
  else if (selectFigure == 'b')
  {
    // Fig 12-9b in the VTK Textbook
    camera->SetPosition(717.356065, -429.889054, -845.381584);
    camera->SetFocalPoint(243.071719, 100.996487, -247.446340);
    camera->SetViewUp(-0.320495, -0.820148, 0.473962);
    camera->SetDistance(929.683631);
    camera->SetClippingRange(293.464446, 1732.794957);
  }
  else if (selectFigure == 'c')
  {
    // Fig 12-9c in the VTK Textbook
    camera->SetPosition(447.560023, -136.611491, -454.753689);
    camera->SetFocalPoint(253.142277, 91.949451, -238.583973);
    camera->SetViewUp(-0.425438, -0.786048, 0.448477);
    camera->SetDistance(369.821187);
    camera->SetClippingRange(0.829116, 829.115939);
  }
  else if (selectFigure == 'd')
  {
    // Fig 12-9d in the VTK Textbook
    camera->SetPosition(347.826249, -469.633647, -236.234262);
    camera->SetFocalPoint(296.893207, 89.307704, -225.156581);
    camera->SetViewUp(-0.687345, -0.076948, 0.722244);
    camera->SetDistance(561.366478);
    camera->SetClippingRange(347.962064, 839.649856);
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
  // Turn off if you do not want it.
  cow->On();
  cow->EnabledOn();

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
  fs::path mhdPath;
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
          mhdPath = fs::path(pth->asString());
        }
        if (pth.name() == "mhd_files")
        {
          for (Json::Value::const_iterator fls =
                   root["files"]["mhd_files"].begin();
               fls != root["files"]["mhd_files"].end(); ++fls)
          {
            fileNames.push_back(fls->asString());
          }
        }
      }
    }
    if (outer.name() == "tissues")
    {
      for (Json::Value::const_iterator tc = root["tissues"]["colors"].begin();
           tc != root["tissues"]["colors"].end(); ++tc)
      {
        parameters.colors[tc.name()] = tc->asString();
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
    if (outer.name() == "tissue_parameters")
    {
      // A map of the parameters and the type of the parameter.
      std::map<std::string, std::string> parameterTypes;
      for (Json::Value::const_iterator p =
               root["tissue_parameters"]["parameter types"].begin();
           p != root["tissue_parameters"]["parameter types"].end(); ++p)
      {
        parameterTypes[p.name()] = p->asString();
      }

      // Map a variant of the correct type to the parameter name.
      // parameterTypes: A map of the parameters and the type of the parameter.
      // p: The Json iterator
      // kv: The map of the parameter name (ket) and
      //     the vatiant of the correct type (value).
      auto populate =
          [&parameterTypes](
              Json::Value::const_iterator& p,
              std::map<std::string, std::variant<int, double, std::string>>&
                  kv) {
            std::string n = p.name();
            if (parameterTypes.find(n) == parameterTypes.end())
            {
              std::cout << "We cannot determine the type for " << n
                        << std::endl;
              return;
            }
            if (parameterTypes[p.name()] == "int")
            {
              kv[ToLowerCase(p.name())] = p->asInt();
            }
            if (parameterTypes[p.name()] == "dbl")
            {
              kv[ToLowerCase(p.name())] = p->asDouble();
            }
            if (parameterTypes[p.name()] == "str")
            {
              kv[ToLowerCase(p.name())] = p->asString();
            }
          };

      std::map<std::string, std::variant<int, double, std::string>> bp;
      for (Json::Value::const_iterator p =
               root["tissue_parameters"]["default"].begin();
           p != root["tissue_parameters"]["default"].end(); ++p)
      {
        populate(p, bp);
      }
      for (Json::Value::const_iterator p =
               root["tissue_parameters"]["frog"].begin();
           p != root["tissue_parameters"]["frog"].end(); ++p)
      {
        populate(p, bp);
      }
      std::set<std::string> exclude{"default", "frog", "parameter types"};
      for (Json::Value::const_iterator p = root["tissue_parameters"].begin();
           p != root["tissue_parameters"].end(); ++p)
      {
        if (exclude.find(p.name()) != exclude.end())
        {
          continue;
        }
        std::map<std::string, std::variant<int, double, std::string>> tmp;
        std::copy(bp.begin(), bp.end(), std::inserter(tmp, tmp.end()));
        for (Json::Value::const_iterator q =
                 root["tissue_parameters"][p.name()].begin();
             q != root["tissue_parameters"][p.name()].end(); ++q)
        {
          populate(q, tmp);
        }
        parameters.tissues[p.name()] = tmp;
        parameters.names.push_back(p.name());
      }
    }
  }

  //  Build and check the paths.
  if (!fileNames.empty())
  {
    if (fileNames.size() != 2)
    {
      std::cerr << "Expected two file names.";
      parameters.parsedOk = false;
    }
    else
    {
      for (size_t i = 0; i < fileNames.size(); i++)
      {
        auto pth = fnPath.parent_path() / mhdPath / fs::path(fileNames[i]);
        fileNames[i] = pth.make_preferred().string();
        if (!(fs::is_regular_file(pth) && fs::exists(pth)))
        {
          std::cerr << "Not a file or path does not exist: " << fileNames[i]
                    << std::endl;
          parameters.parsedOk = false;
        }
        else
        {
          parameters.mhdFiles[pth.stem().string()] =
              pth.make_preferred().string();
        }
      }
    }
  }
  else
  {
    std::cerr << "Expected .mhd file names in the JSON file.";
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
  for (auto const p : this->transform)
  {
    PrintTransform(p.first);
  }
}

vtkSmartPointer<vtkTransform> SliceOrder::Get(std::string const& sliceOrder)
{
  return this->transform[sliceOrder];
}

void CreateTissueActor(
    std::string const& name,
    std::map<std::string, std::variant<int, double, std::string>>& tissue,
    std::map<std::string, std::string>& files, bool const& flying_edges,
    bool const& decimate, vtkLookupTable& color_lut, SliceOrder& so,
    vtkActor* actor)
{

  auto pixelSize = *std::get_if<int>(&tissue["pixel_size"]);
  auto spacing = *std::get_if<double>(&tissue["spacing"]);
  auto startSlice = *std::get_if<int>(&tissue["start_slice"]);
  std::array<double, 3> dataSpacing{static_cast<double>(pixelSize),
                                    static_cast<double>(pixelSize), spacing};
  auto columns = *std::get_if<int>(&tissue["columns"]);
  auto rows = *std::get_if<int>(&tissue["rows"]);
  std::array<double, 3> dataOrigin = {-(columns / 2.0) * pixelSize,
                                      -(rows / 2.0) * pixelSize,
                                      startSlice * spacing};
  std::array<int, 6> voi;
  voi[0] = *std::get_if<int>(&tissue["start_column"]);
  voi[1] = *std::get_if<int>(&tissue["end_column"]);
  voi[2] = *std::get_if<int>(&tissue["start_row"]);
  voi[3] = *std::get_if<int>(&tissue["end_row"]);
  voi[4] = *std::get_if<int>(&tissue["start_slice"]);
  voi[5] = *std::get_if<int>(&tissue["end_slice"]);
  // Adjust y bounds for PNM coordinate system.
  auto tmp = voi[2];
  voi[2] = rows - voi[3] - 1;
  voi[3] = rows - tmp - 1;

  std::string fn;
  if (name == "skin")
  {
    fn = files["frog"];
  }
  else
  {
    fn = files["frogtissue"];
  }

  vtkNew<vtkMetaImageReader> reader;
  reader->SetFileName(fn.c_str());
  reader->SetDataSpacing(dataSpacing.data());
  reader->SetDataOrigin(dataOrigin.data());
  reader->SetDataExtent(voi.data());
  reader->Update();

  // These are used to determine what filters
  // to use based on the options chosen.
  auto islandRemoverFlag{false};
  auto selectTissueFlag{false};
  auto gaussianFlag{false};

  vtkNew<vtkImageIslandRemoval2D> islandRemover;
  vtkNew<vtkImageThreshold> selectTissue;
  if (name != "skin")
  {
    auto ir = *std::get_if<double>(&tissue["island_replace"]);
    auto ia = *std::get_if<double>(&tissue["island_area"]);
    auto idx = *std::get_if<int>(&tissue["tissue"]);
    if (ir > 0)
    {
      islandRemover->SetAreaThreshold(ia);
      islandRemover->SetIslandValue(ir);
      islandRemover->SetReplaceValue(idx);
      islandRemover->SetInputConnection(reader->GetOutputPort());
      islandRemover->Update();
      islandRemoverFlag = true;
    }
    selectTissue->ThresholdBetween(idx, idx);
    selectTissue->SetInValue(255);
    selectTissue->SetOutValue(0);
    if (islandRemoverFlag)
    {
      selectTissue->SetInputConnection(islandRemover->GetOutputPort());
    }
    else
    {
      selectTissue->SetInputConnection(reader->GetOutputPort());
    }
    selectTissue->Update();
    selectTissueFlag = true;
  }

  std::array<int, 3> sampleRate;
  sampleRate[0] = *std::get_if<int>(&tissue["sample_rate_column"]);
  sampleRate[1] = *std::get_if<int>(&tissue["sample_rate_row"]);
  sampleRate[2] = *std::get_if<int>(&tissue["sample_rate_slice"]);
  vtkNew<vtkImageShrink3D> shrinker;
  if (selectTissueFlag)
  {
    shrinker->SetInputConnection(selectTissue->GetOutputPort());
  }
  else
  {
    shrinker->SetInputConnection(reader->GetOutputPort());
  }
  shrinker->SetShrinkFactors(sampleRate.data());
  shrinker->AveragingOn();
  shrinker->Update();

  std::array<double, 3> gsd;
  gsd[0] = *std::get_if<double>(&tissue["gaussian_standard_deviation_column"]);
  gsd[1] = *std::get_if<double>(&tissue["gaussian_standard_deviation_row"]);
  gsd[2] = *std::get_if<double>(&tissue["gaussian_standard_deviation_slice"]);

  vtkNew<vtkImageGaussianSmooth> gaussian;

  bool allZero =
      std::all_of(gsd.begin(), gsd.end(), [](double i) { return i == 0; });
  if (!allZero)
  {
    std::array<double, 3> grf;
    grf[0] = *std::get_if<double>(&tissue["gaussian_radius_factor_column"]);
    grf[1] = *std::get_if<double>(&tissue["gaussian_radius_factor_row"]);
    grf[2] = *std::get_if<double>(&tissue["gaussian_radius_factor_slice"]);

    gaussian->SetStandardDeviations(gsd.data());
    gaussian->SetRadiusFactors(grf.data());
    gaussian->SetInputConnection(shrinker->GetOutputPort());
    gaussian->Update();
    gaussianFlag = true;
  }

  auto iso_value = *std::get_if<double>(&tissue["value"]);
  vtkNew<vtkFlyingEdges3D> flyingIsoSurface;
  vtkNew<vtkMarchingCubes> marchingIsoSurface;
  if (flying_edges)
  {
    if (gaussianFlag)
    {
      flyingIsoSurface->SetInputConnection(gaussian->GetOutputPort());
    }
    else
    {
      flyingIsoSurface->SetInputConnection(shrinker->GetOutputPort());
    }
    flyingIsoSurface->ComputeScalarsOff();
    flyingIsoSurface->ComputeGradientsOff();
    flyingIsoSurface->ComputeNormalsOff();
    flyingIsoSurface->SetValue(0, iso_value);
    flyingIsoSurface->Update();
  }
  else
  {
    if (gaussianFlag)
    {
      marchingIsoSurface->SetInputConnection(gaussian->GetOutputPort());
    }
    else
    {
      marchingIsoSurface->SetInputConnection(shrinker->GetOutputPort());
    }
    marchingIsoSurface->ComputeScalarsOff();
    marchingIsoSurface->ComputeGradientsOff();
    marchingIsoSurface->ComputeNormalsOff();
    marchingIsoSurface->SetValue(0, iso_value);
    marchingIsoSurface->Update();
  }

  auto sliceOrder = *std::get_if<std::string>(&tissue["slice_order"]);
  auto transform = vtkSmartPointer<vtkTransform>::New();
  transform = so.Get(sliceOrder);
  vtkNew<vtkTransformPolyDataFilter> tf;
  tf->SetTransform(transform);
  if (flying_edges)
  {
    tf->SetInputConnection(flyingIsoSurface->GetOutputPort());
  }
  else
  {
    tf->SetInputConnection(marchingIsoSurface->GetOutputPort());
  }

  vtkNew<vtkDecimatePro> decimator;
  if (decimate)
  {
    auto decimateAngle = *std::get_if<double>(&tissue["decimate_angle"]);
    auto decimateError = *std::get_if<double>(&tissue["decimate_error"]);
    auto decimateReduction =
        *std::get_if<double>(&tissue["decimate_reduction"]);
    decimator->SetInputConnection(tf->GetOutputPort());
    decimator->SetFeatureAngle(decimateAngle);
    decimator->PreserveTopologyOn();
    decimator->SetErrorIsAbsolute(1);
    decimator->SetAbsoluteError(decimateError);
    decimator->SetTargetReduction(decimateReduction);
  }

  vtkNew<vtkWindowedSincPolyDataFilter> smoother;
  auto smoothIterations = *std::get_if<int>(&tissue["smooth_iterations"]);
  if (smoothIterations != 0)
  {
    auto smoothAngle = *std::get_if<double>(&tissue["smooth_angle"]);
    auto smoothFactor = *std::get_if<double>(&tissue["smooth_factor"]);
    if (decimate)
    {
      smoother->SetInputConnection(decimator->GetOutputPort());
    }
    else
    {
      smoother->SetInputConnection(tf->GetOutputPort());
    }
    smoother->SetNumberOfIterations(smoothIterations);
    smoother->BoundarySmoothingOff();
    smoother->FeatureEdgeSmoothingOff();
    smoother->SetFeatureAngle(smoothAngle);
    smoother->SetPassBand(smoothFactor);
    smoother->NonManifoldSmoothingOn();
    smoother->NormalizeCoordinatesOff();
    smoother->Update();
  }

  auto featureAngle = *std::get_if<double>(&tissue["feature_angle"]);
  vtkNew<vtkPolyDataNormals> normals;
  if (smoothIterations != 0)
  {
    normals->SetInputConnection(smoother->GetOutputPort());
  }
  else
  {
    if (decimate)
    {
      normals->SetInputConnection(decimator->GetOutputPort());
    }
    else
    {
      normals->SetInputConnection(tf->GetOutputPort());
    }
  }
  normals->SetFeatureAngle(featureAngle);

  vtkNew<vtkStripper> stripper;
  stripper->SetInputConnection(normals->GetOutputPort());

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(stripper->GetOutputPort());

  auto opacity = *std::get_if<double>(&tissue["opacity"]);
  auto tissue_color =
      color_lut.GetTableValue(*std::get_if<int>(&tissue["tissue"]));
  actor->SetMapper(mapper);
  actor->GetProperty()->SetOpacity(opacity);
  actor->GetProperty()->SetDiffuseColor(tissue_color);
  actor->GetProperty()->SetSpecular(0.5);
  actor->GetProperty()->SetSpecularPower(10);
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

} // namespace
