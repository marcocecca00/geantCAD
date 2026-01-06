#include "MeasurementTool.hh"
#include <cmath>
#include <QGroupBox>

#ifndef GEANTCAD_NO_VTK
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#endif

namespace geantcad {

MeasurementTool::MeasurementTool(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
}

#ifndef GEANTCAD_NO_VTK
void MeasurementTool::setRenderer(vtkRenderer* renderer) {
    renderer_ = renderer;
}
#endif

void MeasurementTool::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(8);
    
    // Title
    QLabel* titleLabel = new QLabel("Measurements", this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 11pt; color: #e0e0e0;");
    mainLayout->addWidget(titleLabel);
    
    // Mode buttons
    QGroupBox* modeGroup = new QGroupBox("Measurement Mode", this);
    QHBoxLayout* modeLayout = new QHBoxLayout(modeGroup);
    modeLayout->setContentsMargins(4, 4, 4, 4);
    
    modeGroup_ = new QButtonGroup(this);
    
    distanceBtn_ = new QPushButton("📏 Distance", this);
    distanceBtn_->setCheckable(true);
    distanceBtn_->setToolTip("Measure distance between two points");
    modeGroup_->addButton(distanceBtn_, static_cast<int>(MeasureMode::Distance));
    modeLayout->addWidget(distanceBtn_);
    
    angleBtn_ = new QPushButton("📐 Angle", this);
    angleBtn_->setCheckable(true);
    angleBtn_->setToolTip("Measure angle between three points");
    modeGroup_->addButton(angleBtn_, static_cast<int>(MeasureMode::Angle));
    modeLayout->addWidget(angleBtn_);
    
    pointBtn_ = new QPushButton("📍 Point", this);
    pointBtn_->setCheckable(true);
    pointBtn_->setToolTip("Get coordinates of a point");
    modeGroup_->addButton(pointBtn_, static_cast<int>(MeasureMode::PointPosition));
    modeLayout->addWidget(pointBtn_);
    
    mainLayout->addWidget(modeGroup);
    
    // Instructions
    instructionLabel_ = new QLabel("Select a measurement mode", this);
    instructionLabel_->setStyleSheet("color: #a0a0a0; font-style: italic;");
    instructionLabel_->setWordWrap(true);
    mainLayout->addWidget(instructionLabel_);
    
    // Status
    statusLabel_ = new QLabel("", this);
    statusLabel_->setStyleSheet("color: #00a8ff;");
    mainLayout->addWidget(statusLabel_);
    
    // Measurement list
    QGroupBox* listGroup = new QGroupBox("Saved Measurements", this);
    QVBoxLayout* listLayout = new QVBoxLayout(listGroup);
    listLayout->setContentsMargins(4, 4, 4, 4);
    
    measurementList_ = new QListWidget(this);
    measurementList_->setMinimumHeight(100);
    listLayout->addWidget(measurementList_);
    
    // List buttons
    QHBoxLayout* listBtnLayout = new QHBoxLayout();
    
    deleteBtn_ = new QPushButton("Delete", this);
    deleteBtn_->setEnabled(false);
    listBtnLayout->addWidget(deleteBtn_);
    
    clearBtn_ = new QPushButton("Clear All", this);
    listBtnLayout->addWidget(clearBtn_);
    
    listLayout->addLayout(listBtnLayout);
    
    mainLayout->addWidget(listGroup);
    mainLayout->addStretch();
    
    // Connect signals
    connect(distanceBtn_, &QPushButton::clicked, this, &MeasurementTool::onDistanceMode);
    connect(angleBtn_, &QPushButton::clicked, this, &MeasurementTool::onAngleMode);
    connect(pointBtn_, &QPushButton::clicked, this, &MeasurementTool::onPointMode);
    connect(clearBtn_, &QPushButton::clicked, this, &MeasurementTool::onClearAll);
    connect(deleteBtn_, &QPushButton::clicked, this, &MeasurementTool::onDeleteSelected);
    connect(measurementList_, &QListWidget::currentRowChanged, this, &MeasurementTool::onMeasurementSelected);
}

void MeasurementTool::setMode(MeasureMode mode) {
    if (currentMode_ == mode) {
        // Toggle off
        currentMode_ = MeasureMode::None;
        pendingPoints_.clear();
        
        distanceBtn_->setChecked(false);
        angleBtn_->setChecked(false);
        pointBtn_->setChecked(false);
        
        instructionLabel_->setText("Select a measurement mode");
        statusLabel_->clear();
    } else {
        currentMode_ = mode;
        pendingPoints_.clear();
        
        switch (mode) {
            case MeasureMode::Distance:
                instructionLabel_->setText("Click on two points to measure distance");
                break;
            case MeasureMode::Angle:
                instructionLabel_->setText("Click on three points to measure angle (vertex is second point)");
                break;
            case MeasureMode::PointPosition:
                instructionLabel_->setText("Click on a point to get its coordinates");
                break;
            default:
                instructionLabel_->setText("Select a measurement mode");
                break;
        }
        statusLabel_->clear();
    }
    
    emit modeChanged(currentMode_);
}

void MeasurementTool::onDistanceMode() {
    setMode(MeasureMode::Distance);
}

void MeasurementTool::onAngleMode() {
    setMode(MeasureMode::Angle);
}

void MeasurementTool::onPointMode() {
    setMode(MeasureMode::PointPosition);
}

void MeasurementTool::addPoint(const QVector3D& point) {
    if (currentMode_ == MeasureMode::None) return;
    
    pendingPoints_.push_back(point);
    
    int requiredPoints = 0;
    switch (currentMode_) {
        case MeasureMode::Distance:
            requiredPoints = 2;
            break;
        case MeasureMode::Angle:
            requiredPoints = 3;
            break;
        case MeasureMode::PointPosition:
            requiredPoints = 1;
            break;
        default:
            return;
    }
    
    statusLabel_->setText(QString("Point %1/%2: (%3, %4, %5)")
        .arg(pendingPoints_.size())
        .arg(requiredPoints)
        .arg(point.x(), 0, 'f', 1)
        .arg(point.y(), 0, 'f', 1)
        .arg(point.z(), 0, 'f', 1));
    
    if (static_cast<int>(pendingPoints_.size()) >= requiredPoints) {
        finishMeasurement();
    }
}

void MeasurementTool::finishMeasurement() {
    Measurement m;
    m.id = nextId_++;
    m.mode = currentMode_;
    m.points = pendingPoints_;
    
    switch (currentMode_) {
        case MeasureMode::Distance: {
            m.value = calculateDistance(pendingPoints_[0], pendingPoints_[1]);
            m.unit = "mm";
            m.description = QString("Distance: %1 mm").arg(m.value, 0, 'f', 2);
            break;
        }
        case MeasureMode::Angle: {
            m.value = calculateAngle(pendingPoints_[0], pendingPoints_[1], pendingPoints_[2]);
            m.unit = "°";
            m.description = QString("Angle: %1°").arg(m.value, 0, 'f', 2);
            break;
        }
        case MeasureMode::PointPosition: {
            const auto& p = pendingPoints_[0];
            m.value = 0;
            m.unit = "mm";
            m.description = QString("Point: (%1, %2, %3) mm")
                .arg(p.x(), 0, 'f', 2)
                .arg(p.y(), 0, 'f', 2)
                .arg(p.z(), 0, 'f', 2);
            break;
        }
        default:
            break;
    }
    
    measurements_.push_back(m);
    
#ifndef GEANTCAD_NO_VTK
    createMeasurementVisualization(m);
#endif
    
    updateMeasurementList();
    
    pendingPoints_.clear();
    statusLabel_->setText(m.description);
    
    emit measurementAdded(m);
}

void MeasurementTool::cancelCurrentMeasurement() {
    pendingPoints_.clear();
    statusLabel_->clear();
}

double MeasurementTool::calculateDistance(const QVector3D& p1, const QVector3D& p2) const {
    return (p2 - p1).length();
}

double MeasurementTool::calculateAngle(const QVector3D& p1, const QVector3D& p2, const QVector3D& p3) const {
    QVector3D v1 = (p1 - p2).normalized();
    QVector3D v2 = (p3 - p2).normalized();
    
    double dot = QVector3D::dotProduct(v1, v2);
    dot = std::max(-1.0, std::min(1.0, static_cast<double>(dot)));
    
    return std::acos(dot) * 180.0 / M_PI;
}

void MeasurementTool::removeMeasurement(int id) {
#ifndef GEANTCAD_NO_VTK
    removeMeasurementVisualization(id);
#endif
    
    measurements_.erase(
        std::remove_if(measurements_.begin(), measurements_.end(),
                       [id](const Measurement& m) { return m.id == id; }),
        measurements_.end());
    
    updateMeasurementList();
    emit measurementRemoved(id);
}

void MeasurementTool::clearAllMeasurements() {
#ifndef GEANTCAD_NO_VTK
    clearMeasurementVisualizations();
#endif
    
    measurements_.clear();
    updateMeasurementList();
    emit measurementsCleared();
}

void MeasurementTool::toggleMeasurementVisibility(int id) {
    for (auto& m : measurements_) {
        if (m.id == id) {
            m.visible = !m.visible;
            
#ifndef GEANTCAD_NO_VTK
            auto it = measurementActors_.find(id);
            if (it != measurementActors_.end()) {
                for (auto& actor : it->second) {
                    actor->SetVisibility(m.visible ? 1 : 0);
                }
            }
#endif
            break;
        }
    }
}

void MeasurementTool::updateDisplay() {
    updateMeasurementList();
}

void MeasurementTool::updateMeasurementList() {
    measurementList_->clear();
    for (const auto& m : measurements_) {
        QString icon;
        switch (m.mode) {
            case MeasureMode::Distance: icon = "📏"; break;
            case MeasureMode::Angle: icon = "📐"; break;
            case MeasureMode::PointPosition: icon = "📍"; break;
            default: icon = "•"; break;
        }
        
        QString text = QString("%1 %2").arg(icon).arg(m.description);
        if (!m.visible) {
            text += " (hidden)";
        }
        
        QListWidgetItem* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, m.id);
        measurementList_->addItem(item);
    }
}

QString MeasurementTool::formatMeasurement(const Measurement& m) const {
    return m.description;
}

void MeasurementTool::onClearAll() {
    clearAllMeasurements();
}

void MeasurementTool::onMeasurementSelected(int row) {
    deleteBtn_->setEnabled(row >= 0);
}

void MeasurementTool::onDeleteSelected() {
    QListWidgetItem* item = measurementList_->currentItem();
    if (item) {
        int id = item->data(Qt::UserRole).toInt();
        removeMeasurement(id);
    }
}

#ifndef GEANTCAD_NO_VTK
void MeasurementTool::createMeasurementVisualization(const Measurement& m) {
    if (!renderer_) return;
    
    std::vector<vtkSmartPointer<vtkActor>> actors;
    
    // Create point markers
    for (const auto& point : m.points) {
        vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
        sphere->SetCenter(point.x(), point.y(), point.z());
        sphere->SetRadius(3.0);
        sphere->SetThetaResolution(16);
        sphere->SetPhiResolution(16);
        
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(sphere->GetOutputPort());
        
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(1.0, 0.5, 0.0);  // Orange
        
        renderer_->AddActor(actor);
        actors.push_back(actor);
    }
    
    // Create lines for distance measurement
    if (m.mode == MeasureMode::Distance && m.points.size() >= 2) {
        vtkSmartPointer<vtkLineSource> line = vtkSmartPointer<vtkLineSource>::New();
        line->SetPoint1(m.points[0].x(), m.points[0].y(), m.points[0].z());
        line->SetPoint2(m.points[1].x(), m.points[1].y(), m.points[1].z());
        
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(line->GetOutputPort());
        
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(1.0, 1.0, 0.0);  // Yellow
        actor->GetProperty()->SetLineWidth(2.0);
        
        renderer_->AddActor(actor);
        actors.push_back(actor);
    }
    
    // Create lines for angle measurement
    if (m.mode == MeasureMode::Angle && m.points.size() >= 3) {
        // Line from first point to vertex
        vtkSmartPointer<vtkLineSource> line1 = vtkSmartPointer<vtkLineSource>::New();
        line1->SetPoint1(m.points[0].x(), m.points[0].y(), m.points[0].z());
        line1->SetPoint2(m.points[1].x(), m.points[1].y(), m.points[1].z());
        
        vtkSmartPointer<vtkPolyDataMapper> mapper1 = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper1->SetInputConnection(line1->GetOutputPort());
        
        vtkSmartPointer<vtkActor> actor1 = vtkSmartPointer<vtkActor>::New();
        actor1->SetMapper(mapper1);
        actor1->GetProperty()->SetColor(0.0, 1.0, 0.0);  // Green
        actor1->GetProperty()->SetLineWidth(2.0);
        
        renderer_->AddActor(actor1);
        actors.push_back(actor1);
        
        // Line from vertex to third point
        vtkSmartPointer<vtkLineSource> line2 = vtkSmartPointer<vtkLineSource>::New();
        line2->SetPoint1(m.points[1].x(), m.points[1].y(), m.points[1].z());
        line2->SetPoint2(m.points[2].x(), m.points[2].y(), m.points[2].z());
        
        vtkSmartPointer<vtkPolyDataMapper> mapper2 = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper2->SetInputConnection(line2->GetOutputPort());
        
        vtkSmartPointer<vtkActor> actor2 = vtkSmartPointer<vtkActor>::New();
        actor2->SetMapper(mapper2);
        actor2->GetProperty()->SetColor(0.0, 1.0, 0.0);  // Green
        actor2->GetProperty()->SetLineWidth(2.0);
        
        renderer_->AddActor(actor2);
        actors.push_back(actor2);
    }
    
    measurementActors_[m.id] = actors;
}

void MeasurementTool::removeMeasurementVisualization(int id) {
    auto it = measurementActors_.find(id);
    if (it != measurementActors_.end()) {
        for (auto& actor : it->second) {
            renderer_->RemoveActor(actor);
        }
        measurementActors_.erase(it);
    }
}

void MeasurementTool::clearMeasurementVisualizations() {
    for (auto& pair : measurementActors_) {
        for (auto& actor : pair.second) {
            renderer_->RemoveActor(actor);
        }
    }
    measurementActors_.clear();
}
#endif

} // namespace geantcad

