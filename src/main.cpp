#include "MainWindow.h"
#include <QApplication>
#include <QSurfaceFormat>

int main(int argc, char *argv[])
{
    // Set OpenGL format for better compatibility
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);
    
    // Set application information
    app.setApplicationName("Tekla-Like CAD");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Structural CAD");

    // Create and show main window
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
