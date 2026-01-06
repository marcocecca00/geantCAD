#include "Outliner.hh"
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QMenu>
#include <QContextMenuEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include <QDropEvent>
#include "../../core/include/VolumeNode.hh"
#include "../../core/include/Shape.hh"

Q_DECLARE_METATYPE(geantcad::VolumeNode*)

namespace geantcad {

Outliner::Outliner(QWidget* parent)
    : QTreeWidget(parent)
    , sceneGraph_(nullptr)
    , rootItem_(nullptr)
{
    setHeaderLabel("Scene");
    setSelectionMode(QAbstractItemView::ExtendedSelection); // Allow multi-select
    setDragDropMode(QAbstractItemView::InternalMove); // Enable drag & drop
    setDefaultDropAction(Qt::MoveAction);
    
    // Enable context menu
    setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Set icons for different shape types
    setIconSize(QSize(16, 16));
    
    connect(this, &QTreeWidget::itemSelectionChanged, this, &Outliner::onItemSelectionChanged);
    connect(this, &QTreeWidget::itemActivated, this, &Outliner::onItemActivated);
    connect(this, &QTreeWidget::customContextMenuRequested, this, &Outliner::showContextMenu);
    connect(this, &QTreeWidget::itemChanged, this, &Outliner::onItemChanged);
    
    // Enable inline editing
    setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
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
    
    // Expand all by default for better visibility
    expandAll();
}

QTreeWidgetItem* Outliner::createTreeItem(VolumeNode* node) {
    if (!node) return nullptr;
    
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0, QString::fromStdString(node->getName()));
    item->setData(0, Qt::UserRole, QVariant::fromValue(node));
    
    // Set icon based on shape type (FreeCAD/Blender style)
    if (node->getShape()) {
        QIcon icon;
        // ShapeType is in geantcad namespace, not Shape::ShapeType
        auto shapeType = node->getShape()->getType();
        if (shapeType == geantcad::ShapeType::Box) {
            icon = style()->standardIcon(QStyle::SP_FileIcon);
        } else if (shapeType == geantcad::ShapeType::Tube) {
            icon = style()->standardIcon(QStyle::SP_DirIcon);
        } else if (shapeType == geantcad::ShapeType::Sphere) {
            icon = style()->standardIcon(QStyle::SP_ComputerIcon);
        } else if (shapeType == geantcad::ShapeType::Cone) {
            icon = style()->standardIcon(QStyle::SP_DriveCDIcon);
        } else if (shapeType == geantcad::ShapeType::Trd) {
            icon = style()->standardIcon(QStyle::SP_FileDialogDetailedView);
        } else {
            icon = style()->standardIcon(QStyle::SP_FileIcon);
        }
        item->setIcon(0, icon);
    } else {
        // Default icon for nodes without shape
        item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    }
    
    // Highlight sensitive detectors with special icon and color
    const auto& sdConfig = node->getSDConfig();
    if (sdConfig.enabled) {
        // Add a second icon overlay or change text color
        QColor sdColor(0, 200, 255); // Cyan color for sensitive detectors
        item->setForeground(0, QBrush(sdColor));
        item->setToolTip(0, QString("Sensitive Detector: %1 (%2)")
                        .arg(QString::fromStdString(sdConfig.type))
                        .arg(QString::fromStdString(sdConfig.collectionName)));
        
        // Optionally add a badge icon
        QIcon sdIcon = style()->standardIcon(QStyle::SP_MessageBoxInformation);
        // Combine shape icon with SD indicator
        // For now, we'll just use the color to indicate SD
    } else {
        // Reset to default color
        item->setForeground(0, QBrush());
        item->setToolTip(0, "");
    }
    
    // Visual feedback: show if node has children
    if (!node->getChildren().empty()) {
        item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    }
    
    nodeToItem_[node] = item;
    
    // Add children recursively
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
    } else {
        emit nodeSelected(nullptr);
    }
}

void Outliner::onItemActivated(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column)
    VolumeNode* node = item->data(0, Qt::UserRole).value<VolumeNode*>();
    if (node) {
        emit nodeActivated(node);
    }
}

void Outliner::onItemChanged(QTreeWidgetItem* item, int column) {
    if (column != 0) return; // Only handle name changes
    
    VolumeNode* node = item->data(0, Qt::UserRole).value<VolumeNode*>();
    if (!node) return;
    
    QString newName = item->text(0).trimmed();
    if (newName.isEmpty()) {
        // Restore original name if empty
        item->setText(0, QString::fromStdString(node->getName()));
        return;
    }
    
    // Update node name
    node->setName(newName.toStdString());
    
    // Emit signal to notify other components
    emit nodeSelected(node);
}

void Outliner::showContextMenu(const QPoint& pos) {
    QTreeWidgetItem* item = itemAt(pos);
    if (!item) return;
    
    VolumeNode* node = item->data(0, Qt::UserRole).value<VolumeNode*>();
    if (!node) return;
    
    QMenu contextMenu(this);
    
    QAction* deleteAction = contextMenu.addAction("Delete", [this, node]() {
        if (sceneGraph_) {
            sceneGraph_->removeVolume(node);
            refresh();
        }
    });
    deleteAction->setIcon(style()->standardIcon(QStyle::SP_TrashIcon));
    
    QAction* duplicateAction = contextMenu.addAction("Duplicate", [this, node]() {
        // Emit signal for MainWindow to handle duplication
        emit nodeActivated(node);
    });
    duplicateAction->setIcon(style()->standardIcon(QStyle::SP_FileDialogNewFolder));
    
    contextMenu.addSeparator();
    
    QAction* renameAction = contextMenu.addAction("Rename", [this, item]() {
        editItem(item, 0);
    });
    renameAction->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    
    contextMenu.exec(mapToGlobal(pos));
}

void Outliner::startDrag(Qt::DropActions supportedActions) {
    QTreeWidgetItem* item = currentItem();
    if (!item) return;
    
    VolumeNode* node = item->data(0, Qt::UserRole).value<VolumeNode*>();
    if (!node || node == sceneGraph_->getRoot()) return; // Can't drag root
    
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData();
    // Store node pointer as text (we'll use it in dropMimeData)
    mimeData->setText(QString::number(reinterpret_cast<quintptr>(node)));
    drag->setMimeData(mimeData);
    
    // Set drag icon
    if (item->icon(0).isNull()) {
        drag->setPixmap(style()->standardPixmap(QStyle::SP_FileIcon));
    } else {
        drag->setPixmap(item->icon(0).pixmap(16, 16));
    }
    
    drag->exec(supportedActions);
}

Qt::DropActions Outliner::supportedDropActions() const {
    return Qt::MoveAction;
}

bool Outliner::dropMimeData(QTreeWidgetItem* parent, int index, const QMimeData* data, Qt::DropAction action) {
    if (action != Qt::MoveAction || !data || !sceneGraph_) {
        return false;
    }
    
    // Get dragged node from mime data
    bool ok;
    quintptr nodePtr = data->text().toULongLong(&ok);
    if (!ok) return false;
    
    VolumeNode* draggedNode = reinterpret_cast<VolumeNode*>(nodePtr);
    if (!draggedNode || draggedNode == sceneGraph_->getRoot()) {
        return false; // Can't move root
    }
    
    // Determine new parent
    VolumeNode* newParent = nullptr;
    if (parent) {
        newParent = parent->data(0, Qt::UserRole).value<VolumeNode*>();
    }
    
    // If no parent item, use root
    if (!newParent) {
        newParent = sceneGraph_->getRoot();
    }
    
    // Prevent moving node into itself or its descendants
    if (draggedNode == newParent || draggedNode->isDescendantOf(newParent)) {
        return false;
    }
    
    // Change parent
    draggedNode->setParent(newParent);
    
    // Refresh tree to reflect changes
    refresh();
    
    // Emit signal to notify other components
    emit nodeSelected(draggedNode);
    
    return true;
}

void Outliner::dropEvent(QDropEvent* event) {
    QTreeWidgetItem* item = itemAt(event->pos());
    
    if (item) {
        // Drop on item
        if (dropMimeData(item, 0, event->mimeData(), event->dropAction())) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    } else {
        // Drop on empty area - move to root
        if (dropMimeData(nullptr, 0, event->mimeData(), event->dropAction())) {
            event->acceptProposedAction();
        } else {
            event->ignore();
        }
    }
}

} // namespace geantcad
