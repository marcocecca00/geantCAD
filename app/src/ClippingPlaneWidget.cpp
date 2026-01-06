#include "ClippingPlaneWidget.hh"
#include <QStyle>

namespace geantcad {

ClippingPlaneWidget::ClippingPlaneWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    
#ifndef GEANTCAD_NO_VTK
    xPlane_ = vtkSmartPointer<vtkPlane>::New();
    yPlane_ = vtkSmartPointer<vtkPlane>::New();
    zPlane_ = vtkSmartPointer<vtkPlane>::New();
    
    xPlane_->SetNormal(1, 0, 0);
    yPlane_->SetNormal(0, 1, 0);
    zPlane_->SetNormal(0, 0, 1);
#endif
}

#ifndef GEANTCAD_NO_VTK
void ClippingPlaneWidget::setRenderer(vtkRenderer* renderer) {
    renderer_ = renderer;
    updatePlanes();
}
#endif

void ClippingPlaneWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(8);
    
    // Title
    QLabel* titleLabel = new QLabel("Clipping Planes", this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 11pt; color: #e0e0e0;");
    mainLayout->addWidget(titleLabel);
    
    // X Plane
    mainLayout->addWidget(createPlaneGroup("X Plane (Red)", PlaneAxis::X,
                                           xPlaneCheck_, xSlider_, xValueLabel_, xFlipBtn_));
    
    // Y Plane
    mainLayout->addWidget(createPlaneGroup("Y Plane (Green)", PlaneAxis::Y,
                                           yPlaneCheck_, ySlider_, yValueLabel_, yFlipBtn_));
    
    // Z Plane
    mainLayout->addWidget(createPlaneGroup("Z Plane (Blue)", PlaneAxis::Z,
                                           zPlaneCheck_, zSlider_, zValueLabel_, zFlipBtn_));
    
    // Reset button
    QPushButton* resetBtn = new QPushButton("Reset All Planes", this);
    connect(resetBtn, &QPushButton::clicked, this, &ClippingPlaneWidget::resetPlanes);
    mainLayout->addWidget(resetBtn);
    
    mainLayout->addStretch();
    
    // Connect signals
    connect(xPlaneCheck_, &QCheckBox::toggled, this, &ClippingPlaneWidget::onXPlaneToggled);
    connect(yPlaneCheck_, &QCheckBox::toggled, this, &ClippingPlaneWidget::onYPlaneToggled);
    connect(zPlaneCheck_, &QCheckBox::toggled, this, &ClippingPlaneWidget::onZPlaneToggled);
    
    connect(xSlider_, &QSlider::valueChanged, this, &ClippingPlaneWidget::onXSliderChanged);
    connect(ySlider_, &QSlider::valueChanged, this, &ClippingPlaneWidget::onYSliderChanged);
    connect(zSlider_, &QSlider::valueChanged, this, &ClippingPlaneWidget::onZSliderChanged);
    
    connect(xFlipBtn_, &QPushButton::clicked, this, &ClippingPlaneWidget::onFlipX);
    connect(yFlipBtn_, &QPushButton::clicked, this, &ClippingPlaneWidget::onFlipY);
    connect(zFlipBtn_, &QPushButton::clicked, this, &ClippingPlaneWidget::onFlipZ);
}

QGroupBox* ClippingPlaneWidget::createPlaneGroup(const QString& title, PlaneAxis /*axis*/,
                                                  QCheckBox*& enableCheck, QSlider*& slider,
                                                  QLabel*& valueLabel, QPushButton*& flipBtn) {
    QGroupBox* group = new QGroupBox(this);
    group->setTitle("");
    
    QVBoxLayout* layout = new QVBoxLayout(group);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);
    
    // Header row
    QHBoxLayout* headerLayout = new QHBoxLayout();
    
    enableCheck = new QCheckBox(title, this);
    enableCheck->setStyleSheet("color: #e0e0e0;");
    headerLayout->addWidget(enableCheck);
    
    headerLayout->addStretch();
    
    flipBtn = new QPushButton("⟷", this);
    flipBtn->setToolTip("Flip plane direction");
    flipBtn->setFixedSize(28, 24);
    flipBtn->setEnabled(false);
    headerLayout->addWidget(flipBtn);
    
    layout->addLayout(headerLayout);
    
    // Slider row
    QHBoxLayout* sliderLayout = new QHBoxLayout();
    
    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(-1000, 1000);
    slider->setValue(0);
    slider->setEnabled(false);
    sliderLayout->addWidget(slider);
    
    valueLabel = new QLabel("0.0", this);
    valueLabel->setFixedWidth(60);
    valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    valueLabel->setStyleSheet("color: #a0a0a0;");
    sliderLayout->addWidget(valueLabel);
    
    layout->addLayout(sliderLayout);
    
    return group;
}

void ClippingPlaneWidget::onXPlaneToggled(bool checked) {
    xEnabled_ = checked;
    xSlider_->setEnabled(checked);
    xFlipBtn_->setEnabled(checked);
    updatePlanes();
    emit planeToggled(PlaneAxis::X, checked);
    emit planeChanged();
}

void ClippingPlaneWidget::onYPlaneToggled(bool checked) {
    yEnabled_ = checked;
    ySlider_->setEnabled(checked);
    yFlipBtn_->setEnabled(checked);
    updatePlanes();
    emit planeToggled(PlaneAxis::Y, checked);
    emit planeChanged();
}

void ClippingPlaneWidget::onZPlaneToggled(bool checked) {
    zEnabled_ = checked;
    zSlider_->setEnabled(checked);
    zFlipBtn_->setEnabled(checked);
    updatePlanes();
    emit planeToggled(PlaneAxis::Z, checked);
    emit planeChanged();
}

void ClippingPlaneWidget::onXSliderChanged(int value) {
    xPosition_ = value;
    xValueLabel_->setText(QString::number(value));
    updatePlanes();
    emit planeChanged();
}

void ClippingPlaneWidget::onYSliderChanged(int value) {
    yPosition_ = value;
    yValueLabel_->setText(QString::number(value));
    updatePlanes();
    emit planeChanged();
}

void ClippingPlaneWidget::onZSliderChanged(int value) {
    zPosition_ = value;
    zValueLabel_->setText(QString::number(value));
    updatePlanes();
    emit planeChanged();
}

void ClippingPlaneWidget::onFlipX() {
    xFlipped_ = !xFlipped_;
    updatePlanes();
    emit planeChanged();
}

void ClippingPlaneWidget::onFlipY() {
    yFlipped_ = !yFlipped_;
    updatePlanes();
    emit planeChanged();
}

void ClippingPlaneWidget::onFlipZ() {
    zFlipped_ = !zFlipped_;
    updatePlanes();
    emit planeChanged();
}

void ClippingPlaneWidget::updatePlanes() {
#ifndef GEANTCAD_NO_VTK
    if (!renderer_) return;
    
    if (xEnabled_) {
        double normal = xFlipped_ ? -1.0 : 1.0;
        xPlane_->SetNormal(normal, 0, 0);
        xPlane_->SetOrigin(xPosition_, 0, 0);
    }
    
    if (yEnabled_) {
        double normal = yFlipped_ ? -1.0 : 1.0;
        yPlane_->SetNormal(0, normal, 0);
        yPlane_->SetOrigin(0, yPosition_, 0);
    }
    
    if (zEnabled_) {
        double normal = zFlipped_ ? -1.0 : 1.0;
        zPlane_->SetNormal(0, 0, normal);
        zPlane_->SetOrigin(0, 0, zPosition_);
    }
    
    // Note: Actual clipping is applied in Viewport3D
#endif
}

bool ClippingPlaneWidget::isPlaneEnabled(PlaneAxis axis) const {
    switch (axis) {
        case PlaneAxis::X: return xEnabled_;
        case PlaneAxis::Y: return yEnabled_;
        case PlaneAxis::Z: return zEnabled_;
        default: return false;
    }
}

double ClippingPlaneWidget::getPlanePosition(PlaneAxis axis) const {
    switch (axis) {
        case PlaneAxis::X: return xPosition_;
        case PlaneAxis::Y: return yPosition_;
        case PlaneAxis::Z: return zPosition_;
        default: return 0.0;
    }
}

bool ClippingPlaneWidget::isFlipped(PlaneAxis axis) const {
    switch (axis) {
        case PlaneAxis::X: return xFlipped_;
        case PlaneAxis::Y: return yFlipped_;
        case PlaneAxis::Z: return zFlipped_;
        default: return false;
    }
}

void ClippingPlaneWidget::setPlaneEnabled(PlaneAxis axis, bool enabled) {
    switch (axis) {
        case PlaneAxis::X:
            xPlaneCheck_->setChecked(enabled);
            break;
        case PlaneAxis::Y:
            yPlaneCheck_->setChecked(enabled);
            break;
        case PlaneAxis::Z:
            zPlaneCheck_->setChecked(enabled);
            break;
        default:
            break;
    }
}

void ClippingPlaneWidget::setPlanePosition(PlaneAxis axis, double position) {
    int value = static_cast<int>(position);
    switch (axis) {
        case PlaneAxis::X:
            xSlider_->setValue(value);
            break;
        case PlaneAxis::Y:
            ySlider_->setValue(value);
            break;
        case PlaneAxis::Z:
            zSlider_->setValue(value);
            break;
        default:
            break;
    }
}

void ClippingPlaneWidget::flipPlane(PlaneAxis axis) {
    switch (axis) {
        case PlaneAxis::X:
            onFlipX();
            break;
        case PlaneAxis::Y:
            onFlipY();
            break;
        case PlaneAxis::Z:
            onFlipZ();
            break;
        default:
            break;
    }
}

void ClippingPlaneWidget::resetPlanes() {
    xPlaneCheck_->setChecked(false);
    yPlaneCheck_->setChecked(false);
    zPlaneCheck_->setChecked(false);
    
    xSlider_->setValue(0);
    ySlider_->setValue(0);
    zSlider_->setValue(0);
    
    xFlipped_ = false;
    yFlipped_ = false;
    zFlipped_ = false;
    
    emit planeChanged();
}

} // namespace geantcad

