#pragma once

#include <QToolBar>

namespace geantcad {

class Toolbar : public QToolBar {
    Q_OBJECT

public:
    explicit Toolbar(QWidget* parent = nullptr);

signals:
    // Shape creation moved to Insert -> Shape menu
    void toolSelect();
    void toolMove();
    void toolRotate();
    void toolScale();
    
    void deleteSelected();
    void duplicateSelected();

private:
    void setupActions();
};

} // namespace geantcad

