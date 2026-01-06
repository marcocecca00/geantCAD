#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QSplitter>
#include "Inspector.hh"
#include "../../core/include/SceneGraph.hh"
#include "../../core/include/CommandStack.hh"

namespace geantcad {

/**
 * Properties Panel: Unifica Inspector e Material selector
 * Mostra tutte le proprietà dell'oggetto selezionato
 */
class PropertiesPanel : public QWidget {
    Q_OBJECT

public:
    explicit PropertiesPanel(QWidget* parent = nullptr);
    ~PropertiesPanel();
    
    void setSceneGraph(SceneGraph* sceneGraph);
    void setCommandStack(CommandStack* commandStack);
    
    Inspector* getInspector() { return inspector_; }

signals:
    void nodeChanged(VolumeNode* node);

private:
    void setupUI();
    
    Inspector* inspector_;
    SceneGraph* sceneGraph_;
    CommandStack* commandStack_;
};

} // namespace geantcad

