#include "Outliner.hh"
#include <QHeaderView>
#include <QTreeWidgetItem>
#include "../../core/include/VolumeNode.hh"

Q_DECLARE_METATYPE(geantcad::VolumeNode*)

namespace geantcad {

Outliner::Outliner(QWidget* parent)
    : QTreeWidget(parent)
    , sceneGraph_(nullptr)
    , rootItem_(nullptr)
{
    setHeaderLabel("Scene");
    setSelectionMode(QAbstractItemView::SingleSelection);
    
    connect(this, &QTreeWidget::itemSelectionChanged, this, &Outliner::onItemSelectionChanged);
    connect(this, &QTreeWidget::itemActivated, this, &Outliner::onItemActivated);
}

void Outliner::setSceneGraph(SceneGraph* sceneGraph) {
    sceneGraph_ = sceneGraph;
    refresh();
}

void Outliner::refresh() {
    clear();
    nodeToItem_.clear();
    rootItem_ = nullptr;
    
    if (!sceneGraph_) return;
    
    buildTree();
}

void Outliner::buildTree() {
    if (!sceneGraph_ || !sceneGraph_->getRoot()) return;
    
    rootItem_ = createTreeItem(sceneGraph_->getRoot());
    addTopLevelItem(rootItem_);
    rootItem_->setExpanded(true);
}

QTreeWidgetItem* Outliner::createTreeItem(VolumeNode* node) {
    if (!node) return nullptr;
    
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, QString::fromStdString(node->getName()));
    item->setData(0, Qt::UserRole, QVariant::fromValue(node));
    
    nodeToItem_[node] = item;
    
    // Add children
    for (auto* child : node->getChildren()) {
        QTreeWidgetItem* childItem = createTreeItem(child);
        item->addChild(childItem);
    }
    
    return item;
}

void Outliner::onItemSelectionChanged() {
    QList<QTreeWidgetItem*> selected = selectedItems();
    if (!selected.isEmpty()) {
        QTreeWidgetItem* item = selected.first();
        VolumeNode* node = item->data(0, Qt::UserRole).value<VolumeNode*>();
        if (node) {
            emit nodeSelected(node);
        }
    }
}

void Outliner::onItemActivated(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column)
    VolumeNode* node = item->data(0, Qt::UserRole).value<VolumeNode*>();
    if (node) {
        emit nodeActivated(node);
    }
}

} // namespace geantcad

