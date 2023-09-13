// need to set VTK_USE_MATLAB_MEX ON

#include <vtkMatlabEngineFilter.h>
#include <vtkNew.h>

int main(int argc, char* argv[])
{
  vtkNew<vtkMatlabEngineFilter> matlabFilter;

  return EXIT_SUCCESS;
}
