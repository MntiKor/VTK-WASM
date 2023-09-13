#include <vtkImageData.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <QApplication>
#include <QColor>
#include <QImage>

// Create a green 50x50 imageData for demonstration purposes
vtkSmartPointer<vtkImageData> createDemoImageData()
{
  vtkNew<vtkImageData> image;
  image->SetDimensions(50, 50, 1);
  image->AllocateScalars(VTK_UNSIGNED_CHAR, 3);

  int width = image->GetDimensions()[0];
  int height = image->GetDimensions()[1];

  for (int y = 0; y < height; y++)
  {
    for (int x = 0; x < width; x++)
    {
      unsigned char* pixel =
          static_cast<unsigned char*>(image->GetScalarPointer(x, y, 0));
      pixel[0] = 0;
      pixel[1] = 255;
      pixel[2] = 0;
    }
  }

  return image;
}

// The actual conversion code
QImage vtkImageDataToQImage(vtkSmartPointer<vtkImageData> imageData)
{
  if (!imageData)
  {
    return QImage();
  }

  /// \todo retrieve just the UpdateExtent
  int width = imageData->GetDimensions()[0];
  int height = imageData->GetDimensions()[1];
  QImage image(width, height, QImage::Format_RGB32);
  QRgb* rgbPtr = reinterpret_cast<QRgb*>(image.bits()) + width * (height - 1);
  unsigned char* colorsPtr =
      reinterpret_cast<unsigned char*>(imageData->GetScalarPointer());

  // Loop over the vtkImageData contents.
  for (int row = 0; row < height; row++)
  {
    for (int col = 0; col < width; col++)
    {
      // Swap the vtkImageData RGB values with an equivalent QColor
      *(rgbPtr++) = QColor(colorsPtr[0], colorsPtr[1], colorsPtr[2]).rgb();
      colorsPtr += imageData->GetNumberOfScalarComponents();
    }

    rgbPtr -= width * 2;
  }

  return image;
}

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  QImage qimage = vtkImageDataToQImage(createDemoImageData());
  qimage.save("qimage.png");

  return EXIT_SUCCESS;
}
