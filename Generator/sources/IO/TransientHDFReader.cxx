#include <cstdlib>
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>
#include <vtkDiscretizableColorTransferFunction.h>
#include <vtkHDFReader.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkLookupTable.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkObject.h>
#include <vtkObjectBase.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>

namespace {
vtkNew<vtkDiscretizableColorTransferFunction> GetCTF();
void Animate(vtkObject* caller, unsigned long eid, void* clientdata,
             void* calldata);
} // namespace

int main(int ac, char** av)
{
  if (ac != 2)
  {
    std::cout << "Usage: " << av[0]
              << " filename.vtkhdf eg. warping_spheres.vtkhdf" << std::endl;
    return EXIT_FAILURE;
  }

  vtkNew<vtkNamedColors> colors;

  // Read the dataset.
  vtkNew<vtkHDFReader> reader;
  reader->SetFileName(av[1]);
  reader->Update();
  std::cout << "number of steps: " << reader->GetNumberOfSteps() << endl;
  auto polydata = vtkPolyData::SafeDownCast(reader->GetOutput());

  // Render the dataset.
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputData(polydata);
  mapper->SetLookupTable(GetCTF());
  mapper->SetScalarModeToUsePointFieldData();
  mapper->SelectColorArray("SpatioTemporalHarmonics");

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

  vtkNew<vtkRenderer> renderer;
  renderer->SetBackground(colors->GetColor3d("Wheat").GetData());
  renderer->UseHiddenLineRemovalOn();
  renderer->AddActor(actor);

  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(renderer);
  renWin->SetWindowName("TransientHDFReader");
  renWin->SetSize(1024, 512);
  renWin->Render();

  // Add the animation callback.
  vtkNew<vtkCallbackCommand> command;
  command->SetCallback(&Animate);
  command->SetClientData(reader);

  // Add the interactor.
  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);
  iren->AddObserver(vtkCommand::TimerEvent, command);
  iren->CreateRepeatingTimer(50);

  vtkNew<vtkInteractorStyleTrackballCamera> istyle;
  iren->SetInteractorStyle(istyle);

  iren->Start();

  return EXIT_SUCCESS;
}

namespace {
vtkNew<vtkDiscretizableColorTransferFunction> GetCTF()
{
  vtkNew<vtkDiscretizableColorTransferFunction> ctf;
  ctf->SetColorSpaceToLab();
  ctf->SetScaleToLinear();

  ctf->AddRGBPoint(-30.3399130649763, 0.0862745098039216, 0.00392156862745098,
                   0.298039215686275);
  ctf->AddRGBPoint(-29.3502559661865, 0.113725, 0.0235294, 0.45098);
  ctf->AddRGBPoint(-28.5283393859863, 0.105882, 0.0509804, 0.509804);
  ctf->AddRGBPoint(-27.958028793335, 0.0392157, 0.0392157, 0.560784);
  ctf->AddRGBPoint(-27.4044914245605, 0.0313725, 0.0980392, 0.6);
  ctf->AddRGBPoint(-26.8677291870117, 0.0431373, 0.164706, 0.639216);
  ctf->AddRGBPoint(-26.096134185791, 0.054902, 0.243137, 0.678431);
  ctf->AddRGBPoint(-25.0729293823242, 0.054902, 0.317647, 0.709804);
  ctf->AddRGBPoint(-23.8148933330084, 0.0509804, 0.396078, 0.741176);
  ctf->AddRGBPoint(-22.9992658665124, 0.0392157, 0.466667, 0.768627);
  ctf->AddRGBPoint(-22.1836384000164, 0.0313725, 0.537255, 0.788235);
  ctf->AddRGBPoint(-21.3323650360107, 0.0313725, 0.615686, 0.811765);
  ctf->AddRGBPoint(-20.4601268768311, 0.0235294, 0.709804, 0.831373);
  ctf->AddRGBPoint(-19.5878868103027, 0.0509804, 0.8, 0.85098);
  ctf->AddRGBPoint(-18.8666133880615, 0.0705882, 0.854902, 0.870588);
  ctf->AddRGBPoint(-18.1956596374512, 0.262745, 0.901961, 0.862745);
  ctf->AddRGBPoint(-17.6085758209229, 0.423529, 0.941176, 0.87451);
  ctf->AddRGBPoint(-16.7027893066406, 0.572549, 0.964706, 0.835294);
  ctf->AddRGBPoint(-16.0989303588867, 0.658824, 0.980392, 0.843137);
  ctf->AddRGBPoint(-15.6628112792969, 0.764706, 0.980392, 0.866667);
  ctf->AddRGBPoint(-15.1931447982788, 0.827451, 0.980392, 0.886275);
  ctf->AddRGBPoint(-14.2705841064453, 0.913725, 0.988235, 0.937255);
  ctf->AddRGBPoint(-13.9854288101196, 1, 1, 0.972549019607843);
  ctf->AddRGBPoint(-13.7002735137939, 0.988235, 0.980392, 0.870588);
  ctf->AddRGBPoint(-13.2809276580811, 0.992156862745098, 0.972549019607843,
                   0.803921568627451);
  ctf->AddRGBPoint(-12.9622249603271, 0.992157, 0.964706, 0.713725);
  ctf->AddRGBPoint(-12.4254627227783, 0.988235, 0.956863, 0.643137);
  ctf->AddRGBPoint(-11.5699977874756, 0.980392, 0.917647, 0.509804);
  ctf->AddRGBPoint(-10.8487224578857, 0.968627, 0.87451, 0.407843);
  ctf->AddRGBPoint(-10.1106739044189, 0.94902, 0.823529, 0.321569);
  ctf->AddRGBPoint(-9.57391166687012, 0.929412, 0.776471, 0.278431);
  ctf->AddRGBPoint(-8.78554153442383, 0.909804, 0.717647, 0.235294);
  ctf->AddRGBPoint(-8.08104133605957, 0.890196, 0.658824, 0.196078);
  ctf->AddRGBPoint(-7.50234400308847, 0.878431, 0.619608, 0.168627);
  ctf->AddRGBPoint(-6.68671653659248, 0.870588, 0.54902, 0.156863);
  ctf->AddRGBPoint(-5.87108907009648, 0.85098, 0.47451, 0.145098);
  ctf->AddRGBPoint(-5.05546160360049, 0.831373, 0.411765, 0.133333);
  ctf->AddRGBPoint(-4.23983413710449, 0.811765, 0.345098, 0.113725);
  ctf->AddRGBPoint(-3.4242066706085, 0.788235, 0.266667, 0.0941176);
  ctf->AddRGBPoint(-2.6085792041125, 0.741176, 0.184314, 0.0745098);
  ctf->AddRGBPoint(-1.79295173761651, 0.690196, 0.12549, 0.0627451);
  ctf->AddRGBPoint(-0.977324271120517, 0.619608, 0.0627451, 0.0431373);
  ctf->AddRGBPoint(-0.214114964008331, 0.54902, 0.027451, 0.0705882);
  ctf->AddRGBPoint(0.456838220357895, 0.470588, 0.0156863, 0.0901961);
  ctf->AddRGBPoint(1.21166050434113, 0.4, 0.00392157, 0.101961);
  ctf->AddRGBPoint(2.28518559486346, 0.188235294117647, 0, 0.0705882352941176);

  ctf->SetNumberOfValues(46);
  ctf->DiscretizeOff();
  return ctf;
}

void Animate(vtkObject* caller, unsigned long eid, void* clientdata,
             void* calldata)
{
  vtkRenderWindowInteractor* interactor =
      vtkRenderWindowInteractor::SafeDownCast(caller);
  vtkHDFReader* reader =
      vtkHDFReader::SafeDownCast(static_cast<vtkObjectBase*>(clientdata));
  reader->SetStep((reader->GetStep() == reader->GetNumberOfSteps() - 1)
                      ? 0
                      : reader->GetStep() + 1);
  std::cout << "Current step: " << reader->GetStep() << std::endl;
  reader->Update();
  interactor->Render();
}
} // namespace
