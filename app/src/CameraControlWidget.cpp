#include "CameraControlWidget.hh"
#include <QGridLayout>
#include <QStyle>

#ifndef GEANTCAD_NO_VTK
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <cmath>
#endif

namespace geantcad {

CameraControlWidget::CameraControlWidget(QWidget* parent)
    : QWidget(parent)
#ifndef GEANTCAD_NO_VTK
    , renderer_(nullptr)
    , camera_(nullptr)
#endif
{
    setupUI();
}

CameraControlWidget::~CameraControlWidget() {
}

void CameraControlWidget::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);
    
    QLabel* titleLabel = new QLabel("Camera", this);
    titleLabel->setStyleSheet("font-weight: 600; font-size: 10pt; padding: 2px;");
    layout->addWidget(titleLabel);
    
    // Grid layout for view buttons (3x3 grid)
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setSpacing(2);
    
    // Top row
    QPushButton* topLeftBtn = new QPushButton("↖", this); // Isometric
    topLeftBtn->setToolTip("Isometric View");
    topLeftBtn->setFixedSize(30, 30);
    connect(topLeftBtn, &QPushButton::clicked, this, &CameraControlWidget::onIsometricView);
    gridLayout->addWidget(topLeftBtn, 0, 0);
    
    QPushButton* topBtn = new QPushButton("↑", this); // Top
    topBtn->setToolTip("Top View (+Y)");
    topBtn->setFixedSize(30, 30);
    connect(topBtn, &QPushButton::clicked, this, &CameraControlWidget::onTopView);
    gridLayout->addWidget(topBtn, 0, 1);
    
    QPushButton* topRightBtn = new QPushButton("", this); // Empty
    topRightBtn->setEnabled(false);
    topRightBtn->setFixedSize(30, 30);
    gridLayout->addWidget(topRightBtn, 0, 2);
    
    // Middle row
    QPushButton* leftBtn = new QPushButton("←", this); // Left
    leftBtn->setToolTip("Left View (-X)");
    leftBtn->setFixedSize(30, 30);
    connect(leftBtn, &QPushButton::clicked, this, &CameraControlWidget::onLeftView);
    gridLayout->addWidget(leftBtn, 1, 0);
    
    QPushButton* centerBtn = new QPushButton("○", this); // Front (or reset)
    centerBtn->setToolTip("Front View (+Z)");
    centerBtn->setFixedSize(30, 30);
    connect(centerBtn, &QPushButton::clicked, this, &CameraControlWidget::onFrontView);
    gridLayout->addWidget(centerBtn, 1, 1);
    
    QPushButton* rightBtn = new QPushButton("→", this); // Right
    rightBtn->setToolTip("Right View (+X)");
    rightBtn->setFixedSize(30, 30);
    connect(rightBtn, &QPushButton::clicked, this, &CameraControlWidget::onRightView);
    gridLayout->addWidget(rightBtn, 1, 2);
    
    // Bottom row
    QPushButton* bottomLeftBtn = new QPushButton("", this); // Empty
    bottomLeftBtn->setEnabled(false);
    bottomLeftBtn->setFixedSize(30, 30);
    gridLayout->addWidget(bottomLeftBtn, 2, 0);
    
    QPushButton* bottomBtn = new QPushButton("↓", this); // Bottom
    bottomBtn->setToolTip("Bottom View (-Y)");
    bottomBtn->setFixedSize(30, 30);
    connect(bottomBtn, &QPushButton::clicked, this, &CameraControlWidget::onBottomView);
    gridLayout->addWidget(bottomBtn, 2, 1);
    
    QPushButton* bottomRightBtn = new QPushButton("", this); // Back
    bottomRightBtn->setToolTip("Back View (-Z)");
    bottomRightBtn->setFixedSize(30, 30);
    connect(bottomRightBtn, &QPushButton::clicked, this, &CameraControlWidget::onBackView);
    gridLayout->addWidget(bottomRightBtn, 2, 2);
    
    layout->addLayout(gridLayout);
    layout->addStretch();
}

#ifndef GEANTCAD_NO_VTK
void CameraControlWidget::setRenderer(vtkRenderer* renderer) {
    renderer_ = renderer;
    if (renderer_) {
        camera_ = renderer_->GetActiveCamera();
    }
}

void CameraControlWidget::setCamera(vtkCamera* camera) {
    camera_ = camera;
}

void CameraControlWidget::setViewDirection(double dx, double dy, double dz, double upx, double upy, double upz) {
    if (!camera_ || !renderer_) return;
    
    // Calculate camera position (far enough to see the scene)
    double distance = 500.0; // Default distance
    double pos[3] = {-dx * distance, -dy * distance, -dz * distance};
    
    camera_->SetPosition(pos);
    camera_->SetFocalPoint(0, 0, 0);
    camera_->SetViewUp(upx, upy, upz);
    camera_->ComputeViewPlaneNormal();
    
    renderer_->ResetCamera();
    // Trigger render update via signal instead of direct access
    // The viewport will handle the actual rendering
    
    emit viewChanged();
}

void CameraControlWidget::onTopView() {
    setViewDirection(0, 1, 0, 0, 0, 1); // Looking down +Y, up is +Z
}

void CameraControlWidget::onBottomView() {
    setViewDirection(0, -1, 0, 0, 0, 1); // Looking up -Y, up is +Z
}

void CameraControlWidget::onFrontView() {
    setViewDirection(0, 0, 1, 0, 1, 0); // Looking along +Z, up is +Y
}

void CameraControlWidget::onBackView() {
    setViewDirection(0, 0, -1, 0, 1, 0); // Looking along -Z, up is +Y
}

void CameraControlWidget::onLeftView() {
    setViewDirection(-1, 0, 0, 0, 1, 0); // Looking along -X, up is +Y
}

void CameraControlWidget::onRightView() {
    setViewDirection(1, 0, 0, 0, 1, 0); // Looking along +X, up is +Y
}

void CameraControlWidget::onIsometricView() {
    // Isometric view: equal angles (45 degrees)
    const double angle = 45.0 * M_PI / 180.0;
    double dx = std::cos(angle);
    double dy = std::cos(angle);
    double dz = std::sin(angle);
    setViewDirection(dx, dy, dz, 0, 0, 1); // Up is +Z
}
#else
void CameraControlWidget::setRenderer(void*) {}
void CameraControlWidget::setCamera(void*) {}
void CameraControlWidget::onTopView() {}
void CameraControlWidget::onBottomView() {}
void CameraControlWidget::onFrontView() {}
void CameraControlWidget::onBackView() {}
void CameraControlWidget::onLeftView() {}
void CameraControlWidget::onRightView() {}
void CameraControlWidget::onIsometricView() {}
#endif

} // namespace geantcad

