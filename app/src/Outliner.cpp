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
    // Set up columns: Name, Visibility
    setColumnCount(2);
    setHeaderLabels({"Scene", "👁"});
    header()->setStretchLastSection(false);
    header()->setSectionResizeMode(COL_NAME, QHeaderView::Stretch);
    header()->setSectionResizeMode(COL_VISIBLE, QHeaderView::Fixed);
    header()->resizeSection(COL_VISIBLE, 30);
    
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
    
    // Enable inline editing only for column 0 (name)
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

namespace {
    QString shapeTypeName(ShapeType type) {
        switch (type) {
            case ShapeType::Box: return "Box";
            case ShapeType::Tube: return "Tube";
            case ShapeType::Sphere: return "Sphere";
            case ShapeType::Cone: return "Cone";
            case ShapeType::Trd: return "Trapezoid";
            case ShapeType::Polycone: return "Polycone";
            case ShapeType::Polyhedra: return "Polyhedra";
            default: return "Unknown";
        }
    }
}

QIcon Outliner::getShapeIcon(ShapeType type) const {
    // Return appropriate icon based on shape type
    // Using emoji-style representations that work well in dark themes
    switch (type) {
        case ShapeType::Box:
            return style()->standardIcon(QStyle::SP_DialogSaveButton); // Box-like icon
        case ShapeType::Tube:
            return style()->standardIcon(QStyle::SP_DriveHDIcon);
        case ShapeType::Sphere:
            return style()->standardIcon(QStyle::SP_ComputerIcon);
        case ShapeType::Cone:
            return style()->standardIcon(QStyle::SP_ArrowUp);
        case ShapeType::Trd:
            return style()->standardIcon(QStyle::SP_TitleBarNormalButton);
        case ShapeType::Polycone:
            return style()->standardIcon(QStyle::SP_DriveNetIcon);
        case ShapeType::Polyhedra:
            return style()->standardIcon(QStyle::SP_DirIcon);
        default:
            return style()->standardIcon(QStyle::SP_FileIcon);
    }
}

QTreeWidgetItem* Outliner::createTreeItem(VolumeNode* node) {
    if (!node) return nullptr;
    
    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(COL_NAME, QString::fromStdString(node->getName()));
    item->setData(COL_NAME, Qt::UserRole, QVariant::fromValue(node));
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable | Qt::ItemIsEditable);
    
    // Set visibility checkbox
    item->setCheckState(COL_VISIBLE, node->isVisible() ? Qt::Checked : Qt::Unchecked);
    item->setToolTip(COL_VISIBLE, "Toggle visibility");
    
    // Set icon based on shape type
    if (node->getShape()) {
        item->setIcon(COL_NAME, getShapeIcon(node->getShape()->getType()));
        
        // Build tooltip with shape info
        QString tooltip = QString("Shape: %1").arg(shapeTypeName(node->getShape()->getType()));
        item->setToolTip(COL_NAME, tooltip);
    } else {
        // Default icon for nodes without shape (groups/containers)
        item->setIcon(COL_NAME, style()->standardIcon(QStyle::SP_DirOpenIcon));
        item->setToolTip(COL_NAME, "Container/Group");
    }
    
    // Highlight sensitive detectors with special color
    const auto& sdConfig = node->getSDConfig();
    if (sdConfig.enabled) {
        QColor sdColor(0, 200, 255); // Cyan color for sensitive detectors
        item->setForeground(COL_NAME, QBrush(sdColor));
        item->setToolTip(COL_NAME, QString("Sensitive Detector: %1 (%2)")
                        .arg(QString::fromStdString(sdConfig.type))
                        .arg(QString::fromStdString(sdConfig.collectionName)));
    }
    
    // Visual feedback for hidden items
    if (!node->isVisible()) {
        item->setForeground(COL_NAME, QBrush(QColor(128, 128, 128))); // Gray for hidden
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
        VolumeNode* node = item->data(COL_NAME, Qt::UserRole).value<VolumeNode*>();
        if (node) {
            emit nodeSelected(node);
        }
    } else {
        emit nodeSelected(nullptr);
    }
}

void Outliner::onItemActivated(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column)
    VolumeNode* node = item->data(COL_NAME, Qt::UserRole).value<VolumeNode*>();
    if (node) {
        emit nodeActivated(node);
    }
}

void Outliner::onItemChanged(QTreeWidgetItem* item, int column) {
    VolumeNode* node = item->data(COL_NAME, Qt::UserRole).value<VolumeNode*>();
    if (!node) return;
    
    if (column == COL_NAME) {
        // Handle name changes
        QString newName = item->text(COL_NAME).trimmed();
        if (newName.isEmpty()) {
            // Restore original name if empty
            item->setText(COL_NAME, QString::fromStdString(node->getName()));
            return;
        }
        
        // Update node name
        node->setName(newName.toStdString());
        
        // Emit signal to notify other components
        emit nodeSelected(node);
    } else if (column == COL_VISIBLE) {
        // Handle visibility changes
        bool visible = (item->checkState(COL_VISIBLE) == Qt::Checked);
        node->setVisible(visible);
        
        // Update item appearance
        if (visible) {
            // Restore normal color
            const auto& sdConfig = node->getSDConfig();
            if (sdConfig.enabled) {
                item->setForeground(COL_NAME, QBrush(QColor(0, 200, 255)));
            } else {
                item->setForeground(COL_NAME, QBrush());
            }
        } else {
            // Gray out hidden items
            item->setForeground(COL_NAME, QBrush(QColor(128, 128, 128)));
        }
        
        // Emit signal to update viewport
        emit nodeVisibilityChanged(node, visible);
    }
}

void Outliner::showContextMenu(const QPoint& pos) {
    QTreeWidgetItem* item = itemAt(pos);
    if (!item) return;
    
    VolumeNode* node = item->data(COL_NAME, Qt::UserRole).value<VolumeNode*>();
    if (!node) return;
    
    QMenu contextMenu(this);
    
    // Visibility toggle
    bool isVisible = node->isVisible();
    QAction* visibilityAction = contextMenu.addAction(
        isVisible ? "Hide" : "Show",
        [this, item, node, isVisible]() {
            item->setCheckState(COL_VISIBLE, isVisible ? Qt::Unchecked : Qt::Checked);
        }
    );
    visibilityAction->setIcon(style()->standardIcon(
        isVisible ? QStyle::SP_DialogCloseButton : QStyle::SP_DialogApplyButton
    ));
    
    contextMenu.addSeparator();
    
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
        editItem(item, COL_NAME);
    });
    renameAction->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
    
    contextMenu.exec(mapToGlobal(pos));
}

void Outliner::startDrag(Qt::DropActions supportedActions) {
    QTreeWidgetItem* item = currentItem();
    if (!item) return;
    
    VolumeNode* node = item->data(COL_NAME, Qt::UserRole).value<VolumeNode*>();
    if (!node || node == sceneGraph_->getRoot()) return; // Can't drag root
    
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData();
    // Store node pointer as text (we'll use it in dropMimeData)
    mimeData->setText(QString::number(reinterpret_cast<quintptr>(node)));
    drag->setMimeData(mimeData);
    
    // Set drag icon
    if (item->icon(COL_NAME).isNull()) {
        drag->setPixmap(style()->standardPixmap(QStyle::SP_FileIcon));
    } else {
        drag->setPixmap(item->icon(COL_NAME).pixmap(16, 16));
    }
    
    drag->exec(supportedActions);
}

Qt::DropActions Outliner::supportedDropActions() const {
    return Qt::MoveAction;
}

bool Outliner::dropMimeData(QTreeWidgetItem* parent, int index, const QMimeData* data, Qt::DropAction action) {
    Q_UNUSED(index)
    
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
        newParent = parent->data(COL_NAME, Qt::UserRole).value<VolumeNode*>();
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
    QTreeWidgetItem* item = itemAt(event->position().toPoint());
    
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
