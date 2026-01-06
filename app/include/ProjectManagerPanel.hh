#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QSplitter>
#include "Outliner.hh"
#include "Inspector.hh"
#include "../../core/include/SceneGraph.hh"

namespace geantcad {

/**
 * Left Panel combining Project Manager (tree view) and Information panel
 * Similar to MRADSIM left panel structure
 */
class ProjectManagerPanel : public QWidget {
    Q_OBJECT

public:
    explicit ProjectManagerPanel(QWidget* parent = nullptr);
    ~ProjectManagerPanel();
    
    void setSceneGraph(SceneGraph* sceneGraph);
    void setInspector(Inspector* inspector);
    
    Outliner* getOutliner() { return outliner_; }

signals:
    void nodeSelected(VolumeNode* node);
    void nodeActivated(VolumeNode* node);

private:
    void setupUI();
    
    QSplitter* splitter_;
    Outliner* outliner_;
    Inspector* inspector_;
};

} // namespace geantcad

