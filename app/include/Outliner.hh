#pragma once

#include <QTreeWidget>
#include "../../core/include/SceneGraph.hh"

namespace geantcad {

class Outliner : public QTreeWidget {
    Q_OBJECT

public:
    explicit Outliner(QWidget* parent = nullptr);
    
    void setSceneGraph(SceneGraph* sceneGraph);
    void refresh();

signals:
    void nodeSelected(VolumeNode* node);
    void nodeActivated(VolumeNode* node);

private slots:
    void onItemSelectionChanged();
    void onItemActivated(QTreeWidgetItem* item, int column);

private:
    void buildTree();
    QTreeWidgetItem* createTreeItem(VolumeNode* node);
    
    SceneGraph* sceneGraph_;
    QTreeWidgetItem* rootItem_;
    
    std::map<VolumeNode*, QTreeWidgetItem*> nodeToItem_;
};

} // namespace geantcad

