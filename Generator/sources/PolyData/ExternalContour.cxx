#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkContourFilter.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkVersion.h>
#include <vtkWindowToImageFilter.h>
#include <vtkXMLPolyDataReader.h>

int main(int argc, char* argv[])
{
  vtkNew<vtkNamedColors> colors;

  vtkSmartPointer<vtkPolyData> data3d;

  if (argc > 1)
  {
    // E.g. Bunny.vtp
    vtkNew<vtkXMLPolyDataReader> reader;
    reader->SetFileName(argv[1]);
    reader->Update();

    data3d = reader->GetOutput();
  }
  else
  {
    vtkNew<vtkSphereSource> source;
    source->SetCenter(0.0, 0.0, 5.0);
    source->SetRadius(2.0);
    source->SetPhiResolution(20.);
    source->SetThetaResolution(20.);
    source->Update();

    data3d = source->GetOutput();
  }

  double bounds_data[6], center_data[3];
  data3d->GetBounds(bounds_data);
  data3d->GetCenter(center_data);

  // Black and white scene with the data in order to print the view.
  vtkNew<vtkPolyDataMapper> mapper_data;
  mapper_data->SetInputData(data3d);

  vtkNew<vtkActor> actor_data;
  actor_data->SetMapper(mapper_data);
  actor_data->GetProperty()->SetColor(colors->GetColor3d("Black").GetData());

  vtkNew<vtkRenderer> tmp_rend;
  tmp_rend->SetBackground(colors->GetColor3d("White").GetData());

  tmp_rend->AddActor(actor_data);
  tmp_rend->ResetCamera();
  tmp_rend->GetActiveCamera()->SetParallelProjection(1);

  vtkNew<vtkRenderWindow> tmp_rW;
  tmp_rW->SetOffScreenRendering(1);
  tmp_rW->AddRenderer(tmp_rend);

  tmp_rW->Render();

  // Get a print of the window
  vtkNew<vtkWindowToImageFilter> windowToImageFilter;
  windowToImageFilter->SetInput(tmp_rW);
#if VTK_MAJOR_VERSION >= 8 || VTK_MAJOR_VERSION == 8 && VTK_MINOR_VERSION >= 90
  windowToImageFilter->SetScale(2); // image quality
#else
  windowToImageFilter->SetMagnification(2); // image quality
#endif
  windowToImageFilter->Update();

  // Extract the silhouette corresponding to the black limit of the image.
  vtkNew<vtkContourFilter> ContFilter;
  ContFilter->SetInputConnection(windowToImageFilter->GetOutputPort());
  ContFilter->SetValue(0, 255);
  ContFilter->Update();

  // Make the contour coincide with the data.
  vtkSmartPointer<vtkPolyData> contour = ContFilter->GetOutput();

  double bounds_contour[6], center_contour[3];
  double trans_x = 0.0, trans_y = 0.0, trans_z = 0.0, ratio_x = 0.0,
         ratio_y = 0.0;
  contour->GetBounds(bounds_contour);

  ratio_x = (bounds_data[1] - bounds_data[0]) /
      (bounds_contour[1] - bounds_contour[0]);
  ratio_y = (bounds_data[3] - bounds_data[2]) /
      (bounds_contour[3] - bounds_contour[2]);

  // Rescale the contour so that it shares the same bounds as the
  // input data.
  vtkNew<vtkTransform> transform1;
  transform1->Scale(ratio_x, ratio_y, 1.0);

  vtkNew<vtkTransformPolyDataFilter> tfilter1;
  tfilter1->SetInputData(contour);
  tfilter1->SetTransform(transform1);
  tfilter1->Update();

  contour = tfilter1->GetOutput();

  // Translate the contour so that it shares the same center as the
  // input data.
  contour->GetCenter(center_contour);
  trans_x = center_data[0] - center_contour[0];
  trans_y = center_data[1] - center_contour[1];
  trans_z = center_data[2] - center_contour[2];

  vtkNew<vtkTransform> transform2;
  transform2->Translate(trans_x, trans_y, trans_z);

  vtkNew<vtkTransformPolyDataFilter> tfilter2;
  tfilter2->SetInputData(contour);
  tfilter2->SetTransform(transform2);
  tfilter2->Update();

  contour = tfilter2->GetOutput();

  // Render the result : Input data + resulting silhouette

  // Updating the color of the data.
  actor_data->GetProperty()->SetColor(
      colors->GetColor3d("MistyRose").GetData());

  // Create a mapper and actor of the silhouette.
  vtkNew<vtkPolyDataMapper> mapper_contour;
  mapper_contour->SetInputData(contour);

  vtkNew<vtkActor> actor_contour;
  actor_contour->SetMapper(mapper_contour);
  actor_contour->GetProperty()->SetLineWidth(2.0);

  // 2 renderers and a render window.
  vtkNew<vtkRenderer> renderer1;
  renderer1->AddActor(actor_data);

  vtkNew<vtkRenderer> renderer2;
  renderer2->AddActor(actor_contour);

  vtkNew<vtkRenderWindow> renderwindow;
  renderwindow->SetSize(400, 400);
  renderwindow->SetWindowName("ExternalContour");
  renderwindow->AddRenderer(renderer1);

  renderer1->SetViewport(0.0, 0.0, 0.5, 1.0);
  renderer1->SetBackground(colors->GetColor3d("DarkSlateGray").GetData());

  renderwindow->AddRenderer(renderer2);
  renderer2->SetViewport(0.5, 0.0, 1.0, 1.0);
  renderer2->SetBackground(colors->GetColor3d("MidnightBlue").GetData());

  vtkNew<vtkInteractorStyleTrackballCamera> style;
  vtkNew<vtkRenderWindowInteractor> iren;

  iren->SetRenderWindow(renderwindow);
  iren->SetInteractorStyle(style);

  renderwindow->Render();
  iren->Start();

  return EXIT_SUCCESS;
}
