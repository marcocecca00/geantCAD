#pragma once

#include <QTreeWidget>
#include <QContextMenuEvent>
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
    void nodeVisibilityChanged(VolumeNode* node, bool visible);

protected:
    void startDrag(Qt::DropActions supportedActions) override;
    void dropEvent(QDropEvent* event) override;
    bool dropMimeData(QTreeWidgetItem* parent, int index, const QMimeData* data, Qt::DropAction action) override;
    Qt::DropActions supportedDropActions() const override;

private slots:
    void onItemSelectionChanged();
    void onItemActivated(QTreeWidgetItem* item, int column);
    void onItemChanged(QTreeWidgetItem* item, int column);
    void showContextMenu(const QPoint& pos);

private:
    void buildTree();
    QTreeWidgetItem* createTreeItem(VolumeNode* node);
    QIcon getShapeIcon(ShapeType type) const;
    
    SceneGraph* sceneGraph_;
    QTreeWidgetItem* rootItem_;
    
    std::map<VolumeNode*, QTreeWidgetItem*> nodeToItem_;
    
    // Column indices
    static const int COL_NAME = 0;
    static const int COL_VISIBLE = 1;
};

} // namespace geantcad

