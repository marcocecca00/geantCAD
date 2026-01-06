#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

#ifndef GEANTCAD_NO_VTK
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#endif

namespace geantcad {

/**
 * Camera Control Widget: Widget per rotazione rapida telecamera con assi
 * Stile CAD (FreeCAD, Blender, etc.)
 */
class CameraControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit CameraControlWidget(QWidget* parent = nullptr);
    ~CameraControlWidget();
    
#ifndef GEANTCAD_NO_VTK
    void setRenderer(vtkRenderer* renderer);
    void setCamera(vtkCamera* camera);
#endif

signals:
    void viewChanged();

private slots:
    void onTopView();
    void onBottomView();
    void onFrontView();
    void onBackView();
    void onLeftView();
    void onRightView();
    void onIsometricView();

private:
    void setupUI();
    
#ifndef GEANTCAD_NO_VTK
    vtkRenderer* renderer_;
    vtkCamera* camera_;
    
    void setViewDirection(double dx, double dy, double dz, double upx, double upy, double upz);
#endif
};

} // namespace geantcad

