#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QPushButton>
#include <QButtonGroup>
#include "PhysicsPanel.hh"
#include "OutputPanel.hh"
#include "ParticleGunPanel.hh"
#include "BuildRunDialog.hh"

namespace geantcad {

/**
 * Right Panel Container with vertical tabs (MRADSIM style)
 * Contains: Materials, Source, Analysis, Physics, Simulation panels
 */
class RightPanelContainer : public QWidget {
    Q_OBJECT

public:
    explicit RightPanelContainer(QWidget* parent = nullptr);
    ~RightPanelContainer();
    
    PhysicsPanel* getPhysicsPanel() { return physicsPanel_; }
    OutputPanel* getOutputPanel() { return outputPanel_; }
    ParticleGunPanel* getParticleGunPanel() { return particleGunPanel_; }

signals:
    void physicsConfigChanged();
    void outputConfigChanged();
    void particleGunConfigChanged();

private slots:
    void onTabClicked(int index);

private:
    void setupUI();
    void setupVerticalTabs();
    
    QHBoxLayout* mainLayout_;
    QVBoxLayout* tabLayout_;
    QStackedWidget* stackedWidget_;
    QButtonGroup* tabButtonGroup_;
    
    // Tab buttons
    QPushButton* materialsTab_;
    QPushButton* sourceTab_;
    QPushButton* analysisTab_;
    QPushButton* physicsTab_;
    QPushButton* simulationTab_;
    
    // Panels
    PhysicsPanel* physicsPanel_;
    OutputPanel* outputPanel_;
    ParticleGunPanel* particleGunPanel_;
    QWidget* simulationPanel_; // Wrapper for BuildRunDialog or new panel
    
    int currentTabIndex_;
};

} // namespace geantcad

