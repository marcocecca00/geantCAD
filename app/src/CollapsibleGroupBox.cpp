#include "CollapsibleGroupBox.hh"
#include <QWidget>

namespace geantcad {

CollapsibleGroupBox::CollapsibleGroupBox(const QString& title, QWidget* parent)
    : QWidget(parent)
    , expanded_(true)
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(2, 2, 2, 2);
    mainLayout_->setSpacing(2);
    
    // Header button
    headerButton_ = new QPushButton(this);
    headerButton_->setText("▼ " + title);
    headerButton_->setCheckable(false);
    headerButton_->setFlat(true);
    headerButton_->setObjectName("collapsibleHeaderButton");
    // Styling is now handled by global QSS stylesheet
    
    connect(headerButton_, &QPushButton::clicked, this, &CollapsibleGroupBox::toggle);
    
    mainLayout_->addWidget(headerButton_);
    
    // Content widget (initially visible)
    contentWidget_ = nullptr;
}

void CollapsibleGroupBox::setContent(QWidget* content) {
    if (contentWidget_) {
        mainLayout_->removeWidget(contentWidget_);
        contentWidget_->setParent(nullptr);
    }
    
    contentWidget_ = content;
    if (contentWidget_) {
        contentWidget_->setParent(this);
        mainLayout_->addWidget(contentWidget_);
        contentWidget_->setVisible(expanded_);
    }
}

void CollapsibleGroupBox::setExpanded(bool expanded) {
    if (expanded_ == expanded) return;
    
    expanded_ = expanded;
    updateButtonText();
    
    if (contentWidget_) {
        contentWidget_->setVisible(expanded_);
    }
}

void CollapsibleGroupBox::toggle() {
    setExpanded(!expanded_);
}

void CollapsibleGroupBox::updateButtonText() {
    if (!headerButton_) return;
    
    QString title = headerButton_->text();
    // Remove arrow prefix if present
    if (title.startsWith("▼ ") || title.startsWith("▶ ")) {
        title = title.mid(2);
    }
    
    headerButton_->setText((expanded_ ? "▼ " : "▶ ") + title);
}

} // namespace geantcad

