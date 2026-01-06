#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QButtonGroup>
#include <QVector3D>
#include <vector>

#ifndef GEANTCAD_NO_VTK
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkLineSource.h>
#include <vtkSphereSource.h>
#endif

namespace geantcad {

/**
 * @brief Measurement tool for distance, angle, and area calculations
 * 
 * Provides interactive measurement capabilities in the 3D viewport.
 */
class MeasurementTool : public QWidget {
    Q_OBJECT

public:
    enum class MeasureMode {
        None,
        Distance,      // Point-to-point distance
        Angle,         // Three-point angle
        PointPosition, // Single point coordinates
        EdgeLength,    // Edge length measurement
        Area           // Surface area (future)
    };
    Q_ENUM(MeasureMode)

    struct Measurement {
        int id;
        MeasureMode mode;
        std::vector<QVector3D> points;
        double value;
        QString unit;
        QString description;
        bool visible = true;
    };

    explicit MeasurementTool(QWidget* parent = nullptr);
    ~MeasurementTool() override = default;

#ifndef GEANTCAD_NO_VTK
    void setRenderer(vtkRenderer* renderer);
#endif

    MeasureMode currentMode() const { return currentMode_; }
    const std::vector<Measurement>& getMeasurements() const { return measurements_; }

signals:
    void modeChanged(MeasureMode mode);
    void measurementAdded(const Measurement& measurement);
    void measurementRemoved(int id);
    void measurementsCleared();
    void pickPointRequested();

public slots:
    void setMode(MeasureMode mode);
    void addPoint(const QVector3D& point);
    void cancelCurrentMeasurement();
    void removeMeasurement(int id);
    void clearAllMeasurements();
    void toggleMeasurementVisibility(int id);
    void updateDisplay();

private slots:
    void onDistanceMode();
    void onAngleMode();
    void onPointMode();
    void onClearAll();
    void onMeasurementSelected(int row);
    void onDeleteSelected();

private:
    void setupUI();
    void finishMeasurement();
    void updateMeasurementList();
    QString formatMeasurement(const Measurement& m) const;
    double calculateDistance(const QVector3D& p1, const QVector3D& p2) const;
    double calculateAngle(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3) const;

#ifndef GEANTCAD_NO_VTK
    void createMeasurementVisualization(const Measurement& m);
    void removeMeasurementVisualization(int id);
    void clearMeasurementVisualizations();
#endif

    // UI elements
    QPushButton* distanceBtn_ = nullptr;
    QPushButton* angleBtn_ = nullptr;
    QPushButton* pointBtn_ = nullptr;
    QPushButton* clearBtn_ = nullptr;
    QPushButton* deleteBtn_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QLabel* instructionLabel_ = nullptr;
    QListWidget* measurementList_ = nullptr;
    QButtonGroup* modeGroup_ = nullptr;

    // State
    MeasureMode currentMode_ = MeasureMode::None;
    std::vector<QVector3D> pendingPoints_;
    std::vector<Measurement> measurements_;
    int nextId_ = 1;

#ifndef GEANTCAD_NO_VTK
    vtkRenderer* renderer_ = nullptr;
    std::map<int, std::vector<vtkSmartPointer<vtkActor>>> measurementActors_;
#endif
};

} // namespace geantcad

