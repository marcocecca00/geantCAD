#pragma once

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QString>

namespace geantcad {

/**
 * CollapsibleGroupBox: Widget custom per sezioni collassabili.
 * Header cliccabile con icona ▼/▶, contenuto nascondibile.
 */
class CollapsibleGroupBox : public QWidget {
    Q_OBJECT

public:
    explicit CollapsibleGroupBox(const QString& title, QWidget* parent = nullptr);
    
    void setContent(QWidget* content);
    void setExpanded(bool expanded);
    bool isExpanded() const { return expanded_; }

private slots:
    void toggle();

private:
    void updateButtonText();

    QPushButton* headerButton_;
    QWidget* contentWidget_;
    QVBoxLayout* mainLayout_;
    bool expanded_;
};

} // namespace geantcad

