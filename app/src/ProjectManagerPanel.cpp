#include "ProjectManagerPanel.hh"
#include "Inspector.hh"
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>

namespace geantcad {

ProjectManagerPanel::ProjectManagerPanel(QWidget* parent)
    : QWidget(parent)
    , inspector_(nullptr)
{
    setupUI();
}

ProjectManagerPanel::~ProjectManagerPanel() {
}

void ProjectManagerPanel::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(4);
    
    // Create splitter for Project Manager (top) and Information (bottom)
    splitter_ = new QSplitter(Qt::Vertical, this);
    
    // Project Manager section (top)
    QGroupBox* projectMgrGroup = new QGroupBox("Project Manager", this);
    QVBoxLayout* pmLayout = new QVBoxLayout(projectMgrGroup);
    pmLayout->setContentsMargins(2, 2, 2, 2);
    
    outliner_ = new Outliner(this);
    outliner_->setMinimumHeight(200);
    pmLayout->addWidget(outliner_);
    
    splitter_->addWidget(projectMgrGroup);
    
    // Information section (bottom) - will be populated with Inspector
    // For now, create placeholder
    QGroupBox* infoGroup = new QGroupBox("Information", this);
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    infoLayout->setContentsMargins(2, 2, 2, 2);
    
    // Inspector will be added here when setInspector is called
    QLabel* placeholder = new QLabel("Select an object to view properties", this);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setStyleSheet("color: #888; padding: 20px;");
    infoLayout->addWidget(placeholder);
    
    splitter_->addWidget(infoGroup);
    
    // Set stretch factors: Project Manager gets more space
    splitter_->setStretchFactor(0, 2);
    splitter_->setStretchFactor(1, 1);
    
    // Set initial sizes
    splitter_->setSizes({300, 200});
    
    mainLayout->addWidget(splitter_);
    
    // Connect outliner signals
    connect(outliner_, &Outliner::nodeSelected, this, &ProjectManagerPanel::nodeSelected);
    connect(outliner_, &Outliner::nodeActivated, this, &ProjectManagerPanel::nodeActivated);
}

void ProjectManagerPanel::setSceneGraph(SceneGraph* sceneGraph) {
    if (outliner_) {
        outliner_->setSceneGraph(sceneGraph);
    }
}

void ProjectManagerPanel::setInspector(Inspector* inspector) {
    if (inspector_ == inspector) return;
    
    inspector_ = inspector;
    
    if (inspector_ && splitter_ && splitter_->count() >= 2) {
        // Remove placeholder and add inspector to second widget
        QWidget* infoWidget = splitter_->widget(1);
        if (infoWidget) {
            QLayout* layout = infoWidget->layout();
            if (layout) {
                // Clear existing widgets
                QLayoutItem* item;
                while ((item = layout->takeAt(0)) != nullptr) {
                    QWidget* widget = item->widget();
                    if (widget) {
                        widget->setParent(nullptr);
                        delete widget;
                    }
                    delete item;
                }
                
                // Add inspector
                inspector_->setParent(infoWidget);
                layout->addWidget(inspector_);
            }
        }
    }
}

} // namespace geantcad

