#include <QApplication>
#include "MainWindow.hh"
#include "../../core/include/VolumeNode.hh"
#include <QMetaType>

Q_DECLARE_METATYPE(geantcad::VolumeNode*)

using namespace geantcad;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Register VolumeNode* type for Qt5 QVariant support
    qRegisterMetaType<VolumeNode*>("VolumeNode*");
    
    app.setApplicationName("GeantCAD");
    app.setOrganizationName("GeantCAD");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}

