#include "PropertiesPanel.hh"
#include <QGroupBox>
#include <QLabel>

namespace geantcad {

PropertiesPanel::PropertiesPanel(QWidget* parent)
    : QWidget(parent)
    , inspector_(nullptr)
    , sceneGraph_(nullptr)
    , commandStack_(nullptr)
{
    setupUI();
}

PropertiesPanel::~PropertiesPanel() {
}

void PropertiesPanel::setupUI() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);
    
    // Title
    QLabel* titleLabel = new QLabel("Properties", this);
    titleLabel->setStyleSheet("font-weight: 600; font-size: 11pt; padding: 4px;");
    layout->addWidget(titleLabel);
    
    // Inspector (contains all object properties: Name, Transform, Shape, Material, SD, Optical)
    inspector_ = new Inspector(this);
    layout->addWidget(inspector_);
    
    layout->addStretch();
}

void PropertiesPanel::setSceneGraph(SceneGraph* sceneGraph) {
    sceneGraph_ = sceneGraph;
    // Inspector doesn't need sceneGraph directly
}

void PropertiesPanel::setCommandStack(CommandStack* commandStack) {
    commandStack_ = commandStack;
    if (inspector_) {
        inspector_->setCommandStack(commandStack);
    }
}

} // namespace geantcad

