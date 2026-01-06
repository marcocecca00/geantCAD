#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QVector3D>

#ifndef GEANTCAD_NO_VTK
#include <vtkSmartPointer.h>
#include <vtkPlane.h>
#include <vtkClipPolyData.h>
#include <vtkRenderer.h>
#endif

namespace geantcad {

/**
 * @brief Widget for controlling clipping planes in the 3D viewport
 * 
 * Provides UI for adding, removing, and adjusting clipping planes
 * to cut through the geometry for analysis.
 */
class ClippingPlaneWidget : public QWidget {
    Q_OBJECT

public:
    enum class PlaneAxis {
        X,
        Y,
        Z,
        Custom
    };
    Q_ENUM(PlaneAxis)

    explicit ClippingPlaneWidget(QWidget* parent = nullptr);
    ~ClippingPlaneWidget() override = default;

#ifndef GEANTCAD_NO_VTK
    void setRenderer(vtkRenderer* renderer);
#endif

    bool isPlaneEnabled(PlaneAxis axis) const;
    double getPlanePosition(PlaneAxis axis) const;
    bool isFlipped(PlaneAxis axis) const;

signals:
    void planeChanged();
    void planeToggled(PlaneAxis axis, bool enabled);

public slots:
    void setPlaneEnabled(PlaneAxis axis, bool enabled);
    void setPlanePosition(PlaneAxis axis, double position);
    void flipPlane(PlaneAxis axis);
    void resetPlanes();

private slots:
    void onXPlaneToggled(bool checked);
    void onYPlaneToggled(bool checked);
    void onZPlaneToggled(bool checked);
    void onXSliderChanged(int value);
    void onYSliderChanged(int value);
    void onZSliderChanged(int value);
    void onFlipX();
    void onFlipY();
    void onFlipZ();

private:
    void setupUI();
    void updatePlanes();
    QGroupBox* createPlaneGroup(const QString& title, PlaneAxis axis,
                                 QCheckBox*& enableCheck, QSlider*& slider,
                                 QLabel*& valueLabel, QPushButton*& flipBtn);

    // UI elements
    QCheckBox* xPlaneCheck_ = nullptr;
    QCheckBox* yPlaneCheck_ = nullptr;
    QCheckBox* zPlaneCheck_ = nullptr;
    
    QSlider* xSlider_ = nullptr;
    QSlider* ySlider_ = nullptr;
    QSlider* zSlider_ = nullptr;
    
    QLabel* xValueLabel_ = nullptr;
    QLabel* yValueLabel_ = nullptr;
    QLabel* zValueLabel_ = nullptr;
    
    QPushButton* xFlipBtn_ = nullptr;
    QPushButton* yFlipBtn_ = nullptr;
    QPushButton* zFlipBtn_ = nullptr;
    
    // Plane state
    bool xEnabled_ = false;
    bool yEnabled_ = false;
    bool zEnabled_ = false;
    
    double xPosition_ = 0.0;
    double yPosition_ = 0.0;
    double zPosition_ = 0.0;
    
    bool xFlipped_ = false;
    bool yFlipped_ = false;
    bool zFlipped_ = false;
    
    double rangeMin_ = -1000.0;
    double rangeMax_ = 1000.0;

#ifndef GEANTCAD_NO_VTK
    vtkRenderer* renderer_ = nullptr;
    vtkSmartPointer<vtkPlane> xPlane_;
    vtkSmartPointer<vtkPlane> yPlane_;
    vtkSmartPointer<vtkPlane> zPlane_;
#endif
};

} // namespace geantcad

