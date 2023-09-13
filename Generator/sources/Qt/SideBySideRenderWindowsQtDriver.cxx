#include <QApplication>
#include <QSurfaceFormat>

#include "SideBySideRenderWindowsQt.h"
#include <QVTKOpenGLNativeWidget.h>

int main(int argc, char** argv)
{
  // needed to ensure appropriate OpenGL context is created for VTK rendering.
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

  // QT Stuff
  QApplication app(argc, argv);

  SideBySideRenderWindowsQt sideBySideRenderWindowsQt;
  sideBySideRenderWindowsQt.show();

  return app.exec();
}
